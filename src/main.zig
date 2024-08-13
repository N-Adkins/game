pub const c = @import("c.zig");
pub const platform = @import("platform/platform.zig");
const std = @import("std");

pub fn main() !void {
    try platform.Current.init();
    defer platform.Current.deinit();
    _ = try @import("renderer/vulkan/vulkan.zig").VulkanState.init();
}
