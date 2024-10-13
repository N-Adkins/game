const std = @import("std");
const glfw = @import("zglfw");
const vk = @import("vulkan");
const log = @import("../log.zig");
const Allocator = std.mem.Allocator;

const required_device_extensions = [_][*:0]const u8{vk.extensions.khr_swapchain.name};
const validation_layers = [_][*:0] const u8{"VK_LAYER_KHRONOS_validation"};
const enable_validation_layers = true;

const apis: []const vk.ApiInfo = &.{
    vk.features.version_1_0,
    vk.extensions.khr_surface,
    vk.extensions.khr_swapchain,
};

const BaseDispatch = vk.BaseWrapper(apis);
const InstanceDispatch = vk.InstanceWrapper(apis);
const DeviceDispatch = vk.DeviceWrapper(apis);

const Instance = vk.InstanceProxy(apis);
const Device = vk.DeviceProxy(apis);

pub const GraphicsContext = struct {
    allocator: Allocator,

    base_dispatch: BaseDispatch,

    instance: Instance,
    logical_device: Device,

    surface: vk.SurfaceKHR,
    physical_device: vk.PhysicalDevice,
    physical_device_properties: vk.PhysicalDeviceProperties,
    physical_device_memory_properties: vk.PhysicalDeviceMemoryProperties,

    graphics_queue: Queue,
    present_queue: Queue,

    debug_messenger: ?vk.DebugUtilsMessengerEXT,
    
    pub fn init(allocator: Allocator, app_name: [*:0]const u8, window: *glfw.Window) !GraphicsContext {
        var self: GraphicsContext = undefined;
        self.allocator = allocator;

        // Had to kind of hack this bit together because my libraries weren't playing nice.
        self.base_dispatch = try BaseDispatch.load(
            @as(*const fn(instance: vk.Instance, procname: [*:0]const u8) ?*const fn() void, 
                @ptrCast(&glfw.getInstanceProcAddress)
            )
        );

        const app_info: vk.ApplicationInfo = .{
            .p_application_name = app_name,
            .application_version = vk.makeApiVersion(0, 0, 0, 0),
            .p_engine_name = app_name,
            .engine_version = vk.makeApiVersion(0, 0, 0, 0),
            .api_version = vk.API_VERSION_1_0,
        };

        const instance_extensions = try getInstanceExtensions(allocator);
        defer allocator.free(instance_extensions);
        
        var instance_info: vk.InstanceCreateInfo = .{
            .p_application_info = &app_info,
            .enabled_extension_count = @intCast(instance_extensions.len),
            .pp_enabled_extension_names = @ptrCast(instance_extensions.ptr),
        };
        if (enable_validation_layers and try checkLayerSupport(self.base_dispatch, allocator)) {
            // This allows validation checking on the instance creation function
            var debug_create_info: vk.DebugUtilsMessengerCreateInfoEXT = undefined;
            populateDebugMessengerInfo(&debug_create_info);
            instance_info.p_next = &debug_create_info;

            instance_info.enabled_layer_count = @intCast(validation_layers.len);
            instance_info.pp_enabled_layer_names = @ptrCast(&validation_layers);
        }

        const instance = try self.base_dispatch.createInstance(&instance_info, null);

        const instance_dispatch = try allocator.create(InstanceDispatch);
        errdefer allocator.destroy(instance_dispatch);

        instance_dispatch.* = try InstanceDispatch.load(
            instance, 
            self.base_dispatch.dispatch.vkGetInstanceProcAddr
        );
        self.instance = Instance.init(instance, instance_dispatch);
        errdefer self.instance.destroyInstance(null);

        self.debug_messenger = try createDebugMessenger(self.base_dispatch, self.instance);

        self.surface = try createSurface(self.instance, window);
        errdefer self.instance.destroySurfaceKHR(self.surface, null);

        const candidate = try pickPhysicalDevice(self.instance, self.surface, allocator);
        self.physical_device = candidate.physical_device;
        self.physical_device_properties = candidate.properties;

        const logical_device = try createLogicalDevice(self.instance, candidate);

        const device_dispatch = try allocator.create(DeviceDispatch);
        errdefer allocator.destroy(device_dispatch);

        device_dispatch.* = try DeviceDispatch.load(logical_device, self.instance.wrapper.dispatch.vkGetDeviceProcAddr);
        self.logical_device = Device.init(logical_device, device_dispatch);
        errdefer self.logical_device.destroyDevice(null);

        self.graphics_queue = Queue.init(self.logical_device, candidate.queues.graphics_family);
        self.present_queue = Queue.init(self.logical_device, candidate.queues.present_family);

        self.physical_device_memory_properties = self.instance.getPhysicalDeviceMemoryProperties(self.physical_device);

        return self;
    }

    pub fn deinit(self: *GraphicsContext) void {
        if (self.debug_messenger) |debug_messenger| {
            destroyDebugUtilsMessengerEXT(self.base_dispatch, self.instance, debug_messenger);
        }

        self.logical_device.destroyDevice(null);
        self.instance.destroySurfaceKHR(self.surface, null);
        self.instance.destroyInstance(null);
    
        self.allocator.destroy(self.logical_device.wrapper);
        self.allocator.destroy(self.instance.wrapper);
    }

    pub fn deviceName(self: *const GraphicsContext) []const u8 {
        return std.mem.sliceTo(&self.physical_device_properties.device_name);
    }
};

const Queue = struct {
    handle: vk.Queue,
    family: u32,

    fn init(device: Device, family: u32) Queue {
        return .{
            .handle = device.getDeviceQueue(family, 0),
            .family = family,
        };
    }
};

const QueueIndices = struct {
    graphics_family: u32,
    present_family: u32,
};

const DeviceCandidate = struct {
    physical_device: vk.PhysicalDevice,
    properties: vk.PhysicalDeviceProperties,
    queues: QueueIndices,
};

fn checkLayerSupport(base_dispatch: BaseDispatch, allocator: Allocator) !bool {
    const available_layers = try base_dispatch.enumerateInstanceLayerPropertiesAlloc(allocator);
    defer allocator.free(available_layers);

    for (validation_layers) |requested_layer| {
        var found_layer = false;

        for (available_layers) |property| {
            if (std.mem.eql(u8, std.mem.span(requested_layer), std.mem.sliceTo(&property.layer_name, 0))) {
                found_layer = true;
                break;
            }
        }

        if (!found_layer) {
            return false;
        }
    }
    
    return true;
}

fn getInstanceExtensions(allocator: Allocator) ![][*:0]const u8 {
    const glfw_extensions = try glfw.getRequiredInstanceExtensions();

    var extensions: std.ArrayListUnmanaged([*:0]const u8) = try .initCapacity(allocator, glfw_extensions.len);
    defer extensions.deinit(allocator);

    for (glfw_extensions) |extension| {
        // should never fail since we init with capacity
        extensions.append(allocator, extension) catch unreachable;
    }

    if (enable_validation_layers) {
        try extensions.append(allocator, vk.extensions.ext_debug_utils.name);
    }

    return allocator.dupe([*:0]const u8, extensions.items);
}

fn createDebugMessenger(base_dispatch: BaseDispatch, instance: Instance) !?vk.DebugUtilsMessengerEXT {
    if (!enable_validation_layers) {
        return null;
    }

    var create_info: vk.DebugUtilsMessengerCreateInfoEXT = undefined;
    populateDebugMessengerInfo(&create_info);
    
    return try createDebugUtilsMessengerEXT(base_dispatch, instance, &create_info);
}

fn createDebugUtilsMessengerEXT(
    base_dispatch: BaseDispatch, 
    instance: Instance,
    create_info: *const vk.DebugUtilsMessengerCreateInfoEXT,
) !vk.DebugUtilsMessengerEXT {
    const maybe_func: ?vk.PfnCreateDebugUtilsMessengerEXT = @ptrCast(base_dispatch.getInstanceProcAddr(instance.handle, "vkCreateDebugUtilsMessengerEXT"));
    if (maybe_func) |func| {
        var debug_messenger: vk.DebugUtilsMessengerEXT = undefined;
        if (func(instance.handle, create_info, null, &debug_messenger) == vk.Result.success) {
            return debug_messenger;
        } else {
            return error.DebugMessengerFailed;
        }
    } else {
        return error.ExtensionNotPresent;
    }
}

fn destroyDebugUtilsMessengerEXT(
    base_dispatch: BaseDispatch, 
    instance: Instance,
    debug_messenger: vk.DebugUtilsMessengerEXT,
) void {
    const maybe_func: ?vk.PfnDestroyDebugUtilsMessengerEXT = @ptrCast(base_dispatch.getInstanceProcAddr(instance.handle, "vkDestroyDebugUtilsMessengerEXT"));
    if (maybe_func) |func| {
        func(instance.handle, debug_messenger, null);
    }
}

fn populateDebugMessengerInfo(create_info: *vk.DebugUtilsMessengerCreateInfoEXT) void {
    create_info.* = .{
        .message_severity = .{
            .verbose_bit_ext = true,
            .info_bit_ext = true,
            .warning_bit_ext = true,
            .error_bit_ext = true,
        },
        .message_type = .{
            .general_bit_ext = true,
            .validation_bit_ext = true,
            .performance_bit_ext = true,
        },
        .pfn_user_callback = &debugCallback,
    };
}

fn debugCallback(
    message_severity: vk.DebugUtilsMessageSeverityFlagsEXT,
    message_type: vk.DebugUtilsMessageTypeFlagsEXT,
    callback_data: ?*const vk.DebugUtilsMessengerCallbackDataEXT,
    user_data: ?*anyopaque,
) callconv(.C) vk.Bool32 {
    _ = message_type;
    _ = user_data;

    if (callback_data.?.p_message) |message| {
        if (message_severity.error_bit_ext) {
            log.err("Vulkan: {s}", .{message});        
        } else if (message_severity.warning_bit_ext) {
            log.warn("Vulkan: {s}", .{message});        
        } else if (message_severity.info_bit_ext) {
            log.info("Vulkan: {s}", .{message});        
        } else {
            log.debug("Vulkan: {s}", .{message});        
        }
    }

    return vk.FALSE;
}

fn createSurface(instance: Instance, window: *glfw.Window) !vk.SurfaceKHR {
    var surface: vk.SurfaceKHR = undefined;
    
    // The libraries again were not playing nice so this is kinda cursed
    const result = glfw.createWindowSurface(
        window, @ptrFromInt(@as(usize, @intFromEnum(instance.handle))), 
        null, 
        @ptrCast(&surface)
    );
    if (result != 0) {
        log.err("Failed to create Vulkan surface", .{});
        return error.SurfaceCreationFailed;
    }

    return surface;
}

fn pickPhysicalDevice(instance: Instance, surface: vk.SurfaceKHR, allocator: Allocator) !DeviceCandidate {
    const devices = try instance.enumeratePhysicalDevicesAlloc(allocator);
    defer allocator.free(devices);

    for (devices) |physical_device| {
        if (try checkSuitability(instance, physical_device, surface, allocator)) |candidate| {
            return candidate;
        }
    }
    
    log.err("Failed to find suitable physical device for Vulkan (no supported GPU)", .{});
    return error.NoSuitableDevice;
}

fn checkSuitability(instance: Instance, physical_device: vk.PhysicalDevice, surface: vk.SurfaceKHR, allocator: Allocator) !?DeviceCandidate {
    if (!try checkExtensionSupport(instance, physical_device, allocator))     {
        return null;
    }

    if (!try checkSurfaceSupport(instance, physical_device, surface)) {
        return null;
    }

    if (try checkQueueSupport(instance, physical_device, surface, allocator)) |queues| {
        const properties = instance.getPhysicalDeviceProperties(physical_device);
        return DeviceCandidate{
            .physical_device = physical_device,
            .properties = properties,
            .queues = queues,
        };
    }

    return null;
}

fn checkExtensionSupport(instance: Instance, physical_device: vk.PhysicalDevice, allocator: Allocator) !bool {
    const properties = try instance.enumerateDeviceExtensionPropertiesAlloc(physical_device, null, allocator);
    defer allocator.free(properties);

    for (required_device_extensions) |extension| {
        for (properties) |property| {
            if (std.mem.eql(u8, std.mem.span(extension), std.mem.sliceTo(&property.extension_name, 0))) {
                break;
            }
        } else {
            return false;
        }
    }

    return true;
}

fn checkSurfaceSupport(instance: Instance, physical_device: vk.PhysicalDevice, surface: vk.SurfaceKHR) !bool {
    var format_count: u32 = undefined;
    _ = try instance.getPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, null);

    var present_mode_count: u32 = undefined;
    _ = try instance.getPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, null);
    
    return format_count > 0 and present_mode_count > 0;
}

fn checkQueueSupport(instance: Instance, physical_device: vk.PhysicalDevice, surface: vk.SurfaceKHR, allocator: Allocator) !?QueueIndices {
    const families = try instance.getPhysicalDeviceQueueFamilyPropertiesAlloc(physical_device, allocator);
    defer allocator.free(families);

    var graphics_family: ?u32 = null;
    var present_family: ?u32 = null;

    for (families, 0..) |properties, i| {
        const family: u32 = @intCast(i);

        if (graphics_family == null and properties.queue_flags.graphics_bit) {
            graphics_family = family;
        }

        if (present_family == null and (try instance.getPhysicalDeviceSurfaceSupportKHR(physical_device, family, surface)) == vk.TRUE) {
            present_family = family;
        }
    }

    if (graphics_family != null and present_family != null) {
        return QueueIndices{
            .graphics_family = graphics_family.?,
            .present_family = present_family.?,
        };
    }

    return null;
}

fn createLogicalDevice(instance: Instance, candidate: DeviceCandidate) !vk.Device {
    const priority = [_]f32{1};
    const queue_create_info = [_]vk.DeviceQueueCreateInfo{
        .{
            .queue_family_index = candidate.queues.graphics_family,
            .queue_count = 1,
            .p_queue_priorities = &priority,
        },
        .{
            .queue_family_index = candidate.queues.present_family,
            .queue_count = 1,
            .p_queue_priorities = &priority,
        },
    };

    const queue_count: u32 = if (candidate.queues.graphics_family == candidate.queues.present_family)
        1
    else
        2;

    const device = instance.createDevice(candidate.physical_device, &.{
        .queue_create_info_count = queue_count,
        .p_queue_create_infos = &queue_create_info,
        .enabled_extension_count = required_device_extensions.len,
        .pp_enabled_extension_names = @ptrCast(&required_device_extensions),
    }, null);

    return device;
}
