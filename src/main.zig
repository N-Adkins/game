const std = @import("std");
const glfw = @import("zglfw");
const GraphicsContext = @import("graphics/context.zig").GraphicsContext;

pub fn main() !void {
    var gpa_alloc: std.heap.GeneralPurposeAllocator(.{}) = .{};
    defer _ = gpa_alloc.deinit();
    const gpa = gpa_alloc.allocator();

    try glfw.init();
    defer glfw.terminate();

    const window = try glfw.Window.create(1280, 720, "Window", null);
    defer window.destroy();

    var graphics = try GraphicsContext.init(gpa, "Game");
    defer graphics.deinit();

    while (!window.shouldClose()) {
        glfw.pollEvents();
        window.swapBuffers();
    }
}
