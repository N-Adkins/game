const std = @import("std");
const c = @import("root").c;
const platform = @import("root").platform;

pub const VulkanState = struct {
    instance: c.VkInstance,

    pub fn init() !VulkanState {
        const instance = try createInstance();
        const state: VulkanState = .{
            .instance = instance,
        };
        return state;
    }

    pub fn createInstance() !c.VkInstance {
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
            return error.Vulkan;
        }

        return instance;
    }
};
