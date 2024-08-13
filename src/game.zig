const std = @import("std");

pub const State = struct {
    gpa: std.mem.Allocator,
    frame_arena: std.mem.Allocator, // Arena allocator that is freed after each frame
};

var state: State = undefined;

pub fn initState(new_state: State) void {
    state = new_state;
}

pub fn getState() *State {
    return &state;
}
