const std = @import("std");
const glfw = @import("zglfw");
const vk = @import("vulkan");
const Allocator = std.mem.Allocator;

const required_device_extensions = [_][*:0]const u8{vk.extensions.khr_swapchain.name};

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
    
    pub fn init(allocator: Allocator, app_name: [*:0]const u8, window: *glfw.Window) !GraphicsContext {
        var self: GraphicsContext = undefined;
        self.allocator = allocator;

        // Had to kind of hack this bit together because my libraries weren't playing nice.
        self.base_dispatch = try BaseDispatch.load(
            @as(*const fn(instance: vk.Instance, procname: [*:0]const u8) ?*const fn() void, 
                @ptrCast(&glfw.getInstanceProcAddress)
            )
        );

        const glfw_extensions = try glfw.getRequiredInstanceExtensions();

        const app_info: vk.ApplicationInfo = .{
            .p_application_name = app_name,
            .application_version = vk.makeApiVersion(0, 0, 0, 0),
            .p_engine_name = app_name,
            .engine_version = vk.makeApiVersion(0, 0, 0, 0),
            .api_version = vk.API_VERSION_1_0,
        };

        const instance = try self.base_dispatch.createInstance(&.{
            .p_application_info = &app_info,
            .enabled_extension_count = @intCast(glfw_extensions.len),
            .pp_enabled_extension_names = @ptrCast(glfw_extensions.ptr),
        }, null);

        const instance_dispatch = try allocator.create(InstanceDispatch);
        errdefer allocator.destroy(instance_dispatch);

        instance_dispatch.* = try InstanceDispatch.load(
            instance, 
            self.base_dispatch.dispatch.vkGetInstanceProcAddr
        );
        self.instance = Instance.init(instance, instance_dispatch);
        errdefer self.instance.destroyInstance(null);

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

fn createSurface(instance: Instance, window: *glfw.Window) !vk.SurfaceKHR {
    
    var surface: vk.SurfaceKHR = undefined;
    
    // The libraries again were not playing nice so this is kinda cursed
    const result = glfw.createWindowSurface(
        window, @ptrFromInt(@as(usize, @intFromEnum(instance.handle))), 
        null, 
        @ptrCast(&surface)
    );
    if (result != 0) {
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
