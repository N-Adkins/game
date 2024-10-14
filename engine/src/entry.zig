const std = @import("std");
const vk = @import("vulkan");
const log = @import("core/log.zig");
const PlatformState = @import("platform/platform.zig").PlatformState;

export fn engine_main() callconv(.C) void {
    main_wrapper() catch |e| {
        log.err("Caught fatal error: {any}", .{e}); 
    };
}

fn main_wrapper() !void {
    var gpa_alloc: std.heap.GeneralPurposeAllocator(.{}) = .{};
    defer _ = gpa_alloc.deinit();
    const gpa = gpa_alloc.allocator();
    _ = gpa;

    var state = try PlatformState.init(1280, 720, 640, 360, "App");
    defer state.deinit();

    while (state.pollEvents()) {}
}
