const std = @import("std");
const builtin = @import("builtin");
const platform = builtin.target.os.tag;

pub const PlatformState = switch (platform) {
    .linux => @import("linux.zig").LinuxState,
    else => @compileError("Unsupported platform"),
};
