const std = @import("std");
const c = @import("root").c;
const game = @import("root").game;
const platform = @import("root").platform;
const renderer = @import("../renderer.zig");
const Error = renderer.Error;

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
        var create_info: c.VkInstanceCreateInfo = .{
            .sType = c.VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = 0,
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
