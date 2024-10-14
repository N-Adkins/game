const std = @import("std");

pub fn main() !void {
    const allocator = std.heap.page_allocator;

    const exe_path = try std.fs.selfExeDirPathAlloc(allocator);
    defer allocator.free(exe_path);

    const lib_path = try std.mem.concat(allocator, u8, &.{
        exe_path,
        "/libengine.so",
    });
    defer allocator.free(lib_path);

    var lib = try std.DynLib.open(lib_path);
    defer lib.close();

    if (lib.lookup(*const fn () callconv(.C) void, "engine_main")) |entry_point| {
        entry_point();
    } else {
        try std.io.getStdErr().writeAll("Failed to find engine entrypoint, missing DLL?\n");
    }
}
