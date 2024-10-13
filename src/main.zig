const std = @import("std");
const glfw = @import("zglfw");
const vk = @import("vulkan");
const log = @import("log.zig");
const GraphicsContext = @import("graphics/context.zig").GraphicsContext;
const Swapchain = @import("graphics/swapchain.zig").Swapchain;

pub fn main() !void {
    var gpa_alloc: std.heap.GeneralPurposeAllocator(.{}) = .{};
    defer _ = gpa_alloc.deinit();
    const gpa = gpa_alloc.allocator();

    try glfw.init();
    defer glfw.terminate();
    
    if (!glfw.isVulkanSupported()) {
        log.err("Failed to find libvulkan", .{});
        return error.VulkanNotSupported;
    }
    
    glfw.windowHint(.client_api, 0);
    const window = try glfw.Window.create(1280, 720, "Window", null);
    defer window.destroy();

    var graphics = try GraphicsContext.init(gpa, "Game", window);
    defer graphics.deinit();

    const extent: vk.Extent2D = .{
        .width = 1280,
        .height = 720,
    };
    var swapchain = try Swapchain.init(&graphics, gpa, extent);
    defer swapchain.deinit();

    while (!window.shouldClose()) {
        glfw.pollEvents();
        window.swapBuffers();
    }
}
