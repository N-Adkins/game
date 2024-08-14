pub const c = @import("c.zig");
pub const game = @import("game.zig");
pub const platform = @import("platform/platform.zig");
const std = @import("std");

pub fn main() !void {
    try platform.Current.init();
    defer platform.Current.deinit();

    var gpa_alloc = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa_alloc.deinit();
    const gpa = gpa_alloc.allocator();

    var frame_arena_alloc = std.heap.ArenaAllocator.init(gpa);
    defer frame_arena_alloc.deinit();
    const frame_arena = frame_arena_alloc.allocator();

    var static_arena_alloc = std.heap.ArenaAllocator.init(gpa);
    defer static_arena_alloc.deinit();
    const static_arena = static_arena_alloc.allocator();

    game.initState(.{
        .gpa = gpa,
        .static_arena = static_arena,
        .frame_arena = frame_arena,
    });

    var renderer = try @import("renderer/vulkan/vulkan.zig").VulkanState.init();
    defer renderer.deinit();
}
