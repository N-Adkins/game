const std = @import("std");
const c = @import("root").c;
const assert = std.debug.assert;

// For the windowing stuff we can migrate later on if we have a need to use platform-specific APIs
// but for now GLFW is fine.

pub const Current = struct {
    var initialized: bool = false;

    pub fn init() !void {
        if (c.glfwInit() != c.GLFW_TRUE) {
            return error.GLFW;
        }
        initialized = true;
    }

    pub fn deinit() void {
        c.glfwTerminate();
        initialized = false;
    }

    pub fn getExtensions() [][*c]const u8 {
        assert(initialized);
        var count: u32 = 0;
        var extensions: [*c][*c]const u8 = null;
        extensions = c.glfwGetRequiredInstanceExtensions(&count);
        return extensions[0..count];
    }
};
