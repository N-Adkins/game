const std = @import("std");
const builtin = @import("builtin");
const c = @import("root").c;
const game = @import("root").game;
const platform = @import("root").platform;
const renderer = @import("../renderer.zig");
const Error = renderer.Error;

const enable_layers: bool = switch (builtin.mode) {
    .Debug, .ReleaseSafe => true,
    .ReleaseFast, .ReleaseSmall => false,
};

const requested_layers = [_][*:0]const u8{
    "VK_LAYER_KHRONOS_validation",
};

pub const VulkanState = struct {
    instance: c.VkInstance,

    pub fn init() Error!VulkanState {
        std.log.info("Initializing Vulkan", .{});
        const instance = try createInstance();
        try logExtensions();
        const state: VulkanState = .{
            .instance = instance,
        };
        return state;
    }

    pub fn deinit(self: *VulkanState) void {
        std.log.info("Deinitializing Vulkan", .{});
        c.vkDestroyInstance(self.instance, null);
    }

    fn createInstance() Error!c.VkInstance {
        var app_info: c.VkApplicationInfo = .{
            .sType = c.VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Game",
            .applicationVersion = c.VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "None",
            .engineVersion = c.VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = c.VK_API_VERSION_1_0,
        };

        const extensions = platform.Current.getExtensions();
        var layer_count: u32 = 0;
        const layer_names: [*c][*c]const u8 = if (enable_layers) blk: {
            try checkLayerSupport();
            layer_count = @truncate(requested_layers.len);
            // Yeah yeah, scary const cast. This is the intended usecase, Vulkan should not be touching
            // that memory.
            break :blk @constCast(@ptrCast(&requested_layers[0]));      
        } else undefined;
        var create_info: c.VkInstanceCreateInfo = .{
            .sType = c.VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = layer_count,
            .ppEnabledLayerNames = layer_names,
            .enabledExtensionCount = @truncate(extensions.len),
            .ppEnabledExtensionNames = extensions.ptr,
        };

        var instance: c.VkInstance = undefined;
        if (c.vkCreateInstance(&create_info, null, &instance) != c.VK_SUCCESS) {
            std.log.err("Failed to create Vulkan instance", .{});
            return Error.Vulkan;
        }

        return instance;
    }

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
};
