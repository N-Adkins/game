const std = @import("std");
const builtin = @import("builtin");
const c = @import("root").c;
const game = @import("root").game;
const platform = @import("root").platform;
const Renderer = @import("../Renderer.zig");
const Error = Renderer.Error;

const enable_layers: bool = switch (builtin.mode) {
    .Debug, .ReleaseSafe => true,
    .ReleaseFast, .ReleaseSmall => false,
};

const requested_layers = [_][*:0]const u8{
    "VK_LAYER_KHRONOS_validation",
};

pub const VulkanState = struct {
    instance: c.VkInstance,
    debug_messenger: c.VkDebugUtilsMessengerEXT,

    pub fn init() Error!VulkanState {
        std.log.info("Initializing Vulkan", .{});
        const instance = try createInstance();
        try logExtensions();
        const debug_messenger = try createDebugMessenger(instance);
        const state: VulkanState = .{
            .instance = instance,
            .debug_messenger = debug_messenger,
        };
        return state;
    }

    pub fn deinit(self: *VulkanState) void {
        std.log.info("Deinitializing Vulkan", .{});
        self.destroyDebugMessenger();
        c.vkDestroyInstance(self.instance, null);
    }

    pub fn renderer(self: *VulkanState) Renderer {
        return .{
            .context = @ptrCast(self),
            .vtable = .{},
        };
    }

    fn createInstance() Error!c.VkInstance {
        const state = game.getState();

        var app_info: c.VkApplicationInfo = .{
            .sType = c.VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Game",
            .applicationVersion = c.VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "None",
            .engineVersion = c.VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = c.VK_API_VERSION_1_0,
        };

        var extensions = platform.Current.getExtensions();
        if (enable_layers) {
            extensions.append(state.frame_arena, c.VK_EXT_DEBUG_UTILS_EXTENSION_NAME) catch {
                @panic("OOM");
            };
        }

        var layer_count: u32 = 0;
        const layer_names: [*c][*c]const u8 = if (enable_layers) blk: {
            try checkLayerSupport();
            layer_count = @truncate(requested_layers.len);
            // Yeah yeah, scary const cast. This is the intended usecase, Vulkan should not be touching
            // that memory.
            break :blk @constCast(@ptrCast(&requested_layers[0]));
        } else null;

        var create_info: c.VkInstanceCreateInfo = .{
            .sType = c.VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = layer_count,
            .ppEnabledLayerNames = layer_names,
            .enabledExtensionCount = @truncate(extensions.items.len),
            .ppEnabledExtensionNames = extensions.items.ptr,
            .pNext = null,
        };

        // We do this to make it so that the createInstance and destroyInstance functions are debugged
        var debug_messenger_info = createDebugMessengerInfo();
        if (enable_layers) {
            create_info.pNext = @as(*c.VkDebugUtilsMessengerCreateInfoEXT, @alignCast(@ptrCast(&debug_messenger_info)));
        }

        var instance: c.VkInstance = undefined;
        if (c.vkCreateInstance(&create_info, null, &instance) != c.VK_SUCCESS) {
            std.log.err("Failed to create Vulkan instance", .{});
            return Error.Vulkan;
        }

        return instance;
    }

    /// Checks all requested validation layers and errors if they are not all supported.
    fn checkLayerSupport() Error!void {
        var state = game.getState();

        var layer_count: u32 = 0;
        if (c.vkEnumerateInstanceLayerProperties(&layer_count, null) != c.VK_SUCCESS) {
            std.log.err("Failed to enumerate instance layer properties", .{});
            return Error.Vulkan;
        }

        const available_layers: []c.VkLayerProperties = state.frame_arena.alloc(c.VkLayerProperties, layer_count) catch {
            @panic("OOM");
        };
        if (c.vkEnumerateInstanceLayerProperties(&layer_count, available_layers.ptr) != c.VK_SUCCESS) {
            std.log.err("Failed to enumerate instance layer properties", .{});
            return Error.Vulkan;
        }

        for (requested_layers) |layer_name| {
            var layer_found = false;
            for (available_layers) |layer_properties| {
                if (std.mem.orderZ(u8, layer_name, @ptrCast(&layer_properties.layerName[0])) == .eq) {
                    layer_found = true;
                    break;
                }
            }
            if (!layer_found) {
                std.log.err("Requested layer {s} but it is not available", .{layer_name});
                return Error.Vulkan;
            }
        }
    }

    /// This just logs all of the Vulkan extensions that are supported on the device.
    fn logExtensions() Error!void {
        var state = game.getState();
        var extension_count: u32 = 0;
        if (c.vkEnumerateInstanceExtensionProperties(null, &extension_count, null) != c.VK_SUCCESS) {
            std.log.err("Failed to enumerate instance extension properties", .{});
            return Error.Vulkan;
        }
        const extensions: []c.VkExtensionProperties = state.frame_arena.alloc(c.VkExtensionProperties, extension_count) catch {
            @panic("OOM");
        };
        if (c.vkEnumerateInstanceExtensionProperties(null, &extension_count, extensions.ptr) != c.VK_SUCCESS) {
            std.log.err("Failed to enumerate instance extension properties", .{});
            return Error.Vulkan;
        }
        std.log.debug("Listing available Vulkan extensions", .{});
        for (extensions) |*extension| {
            std.log.debug("{s}", .{extension.extensionName});
        }
    }

    fn createDebugMessengerInfo() c.VkDebugUtilsMessengerCreateInfoEXT {
        return .{
            .sType = c.VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = c.VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | c.VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | c.VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = @alignCast(@ptrCast(&debugCallback)),
            .pUserData = null,
        };
    }

    fn createDebugMessenger(instance: c.VkInstance) Error!c.VkDebugUtilsMessengerEXT {
        std.log.info("Initializing debug messenger for Vulkan", .{});

        const create_info = createDebugMessengerInfo();

        var messenger: c.VkDebugUtilsMessengerEXT = undefined;

        const maybe_func: c.PFN_vkCreateDebugUtilsMessengerEXT = @alignCast(@ptrCast(c.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")));

        const failed = blk: {
            if (maybe_func) |func| {
                if (func(instance, &create_info, null, &messenger) != c.VK_SUCCESS) {
                    break :blk true;
                }
                break :blk false;
            } else break :blk true;
        };

        if (failed) {
            std.log.err("Failed to set up debug messenger", .{});
            return Error.Vulkan;
        }

        return messenger;
    }

    fn destroyDebugMessenger(self: *VulkanState) void {
        std.log.info("Deinitializing debug messenger for Vulkan", .{});
        const maybe_func: c.PFN_vkDestroyDebugUtilsMessengerEXT = @alignCast(@ptrCast(c.vkGetInstanceProcAddr(self.instance, "vkDestroyDebugUtilsMessengerEXT")));
        if (maybe_func) |func| {
            return func(self.instance, self.debug_messenger, null);
        }
    }

    fn debugCallback(
        severity: c.VkDebugUtilsMessageSeverityFlagBitsEXT,
        message_type: c.VkDebugUtilsMessageTypeFlagsEXT,
        callback_data: *const c.VkDebugUtilsMessengerCallbackDataEXT,
        user_data: *const anyopaque,
    ) callconv(.C) c.VkBool32 {
        _ = message_type;
        _ = user_data;
        switch (severity) {
            c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT => std.log.debug("{s}", .{callback_data.pMessage}),
            c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT => std.log.info("{s}", .{callback_data.pMessage}),
            c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT => std.log.warn("{s}", .{callback_data.pMessage}),
            c.VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT => std.log.err("{s}", .{callback_data.pMessage}),
            else => unreachable,
        }
        return c.VK_FALSE;
    }
};
