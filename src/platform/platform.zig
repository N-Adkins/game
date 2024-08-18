const std = @import("std");
const c = @import("root").c;
const game = @import("root").game;
const assert = std.debug.assert;

// For the windowing stuff we can migrate later on if we have a need to use platform-specific APIs
// but for now GLFW is fine.

pub const Current = struct {
    var initialized: bool = false;

    pub fn init() !void {
        std.log.info("Initializing Platform", .{});
        if (c.glfwInit() != c.GLFW_TRUE) {
            std.log.err("Failed to initialize GLFW", .{});
            return error.GLFW;
        }
        initialized = true;
    }

    pub fn deinit() void {
        std.log.info("Deinitializing Platform", .{});
        c.glfwTerminate();
        initialized = false;
    }

    /// The array outputted by this is on the frame arena so no need to free it.
    pub fn getExtensions() std.ArrayListUnmanaged([*c]const u8) {
        assert(initialized);
        const state = game.getState();
        var count: u32 = 0;
        var extensions: [*c][*c]const u8 = c.glfwGetRequiredInstanceExtensions(&count);
        var array = std.ArrayListUnmanaged([*c]const u8).initCapacity(state.frame_arena, count) catch {
            @panic("OOM");
        };
        for (extensions[0..count]) |extension| {
            array.append(state.frame_arena, extension) catch {
                @panic("OOM");
            };
        }
        return array;
    }
};
