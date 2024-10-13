const std = @import("std");

const Logger = struct {
    const allocator = std.heap.c_allocator;

    mutex: std.Thread.Mutex = .{},
    level: LogLevel = .debug,

    fn setLevel(self: *Logger, level: LogLevel) void {
        self.mutex.lock();
        defer self.mutex.unlock();
        self.level = level;
    }

    fn log(
        self: *Logger, 
        comptime level: LogLevel, 
        comptime fmt: []const u8, 
        args: anytype
    ) void {
        const level_str: []const u8 = comptime switch(level) {
            .debug => "Debug",
            .info => "Info",
            .warning => "Warning",
            .err => "Error",
        };
        
        self.mutex.lock();
        defer self.mutex.unlock();

        if (@intFromEnum(level) < @intFromEnum(self.level)) {
            return;
        }

        const message = std.fmt.allocPrint(allocator, "[" ++ level_str ++ "]: " ++ fmt ++ "\n", args) catch @panic("OOM while logging");
        defer allocator.free(message);
        std.io.getStdErr().writeAll(message) catch @panic("OOM while logging");
    }
};

const LogLevel = enum(u8) {
    debug = 0,
    info = 1,
    warning = 2,
    err = 3,
};

var GLOBAL_LOGGER: Logger = .{};

pub inline fn setLevel(level: LogLevel) void {
    GLOBAL_LOGGER.setLevel(level);
}

pub inline fn debug(comptime fmt: []const u8, args: anytype) void {
    GLOBAL_LOGGER.log(.debug, fmt, args);
}

pub inline fn info(comptime fmt: []const u8, args: anytype) void {
    GLOBAL_LOGGER.log(.info, fmt, args);
}

pub inline fn warn(comptime fmt: []const u8, args: anytype) void {
    GLOBAL_LOGGER.log(.warning, fmt, args);
}

pub inline fn err(comptime fmt: []const u8, args: anytype) void {
    GLOBAL_LOGGER.log(.err, fmt, args);
}
