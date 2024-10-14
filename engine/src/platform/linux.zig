const std = @import("std");
const log = @import("../core/log.zig");

const c = @cImport({
    @cInclude("xcb/xcb.h");
    @cInclude("X11/keysym.h");
    @cInclude("X11/XKBlib.h");
    @cInclude("X11/Xlib.h");
    @cInclude("X11/Xlib-xcb.h");
});

pub const LinuxState = struct {
    display: *c.Display,
    connection: *c.xcb_connection_t,
    window: c.xcb_window_t,
    screen: *c.xcb_screen_t,
    wm_protocols: c.xcb_atom_t,
    wm_delete_win: c.xcb_atom_t,

    pub fn init(width: u32, height: u32, x: u32, y: u32, app_name: [*:0]const u8) !LinuxState {
        var state: LinuxState = undefined;

        // Connecting to the X server
        state.display = c.XOpenDisplay(null) orelse {
            log.err("Failed to connect to the X server", .{});
            return error.XFailedToConnect;
        };

        // Turning off key repeating
        _ = c.XAutoRepeatOn(state.display);

        // Grabbing the actual connection to the server
        state.connection = c.XGetXCBConnection(state.display) orelse {
            log.err("Failed to fetch connection to the X server", .{});
            return error.XFailedToFetchConnection;
        };

        if (c.xcb_connection_has_error(state.connection) != 0) {
             log.err("X server connection has an error", .{});
            return error.XConnectionError;
        }
        
        // Fetching X data from server
        const setup: *const c.xcb_setup_t = c.xcb_get_setup(state.connection) orelse {
            log.err("Failed to fetch setup data from X server", .{});
            return error.XFailedToFetchSetup; 
        };
        
        // Looping through all of the screens
        var it: c.xcb_screen_iterator_t = c.xcb_setup_roots_iterator(setup);
        var screen_p: c_int = 0;
        while (screen_p > 0) {
            c.xcb_screen_next(&it);
            screen_p -= 1;
        }

        // Assign the screen after iterating
        state.screen = @ptrCast(it.data);
        
        // Assigning an ID for the window be made on
        state.window = c.xcb_generate_id(state.connection);

        const event_mask = c.XCB_CW_BACK_PIXEL | c.XCB_CW_EVENT_MASK;
        const event_values = 
            c.XCB_EVENT_MASK_BUTTON_PRESS | 
            c.XCB_EVENT_MASK_BUTTON_RELEASE |
            c.XCB_EVENT_MASK_KEY_PRESS |
            c.XCB_EVENT_MASK_KEY_RELEASE |
            c.XCB_EVENT_MASK_EXPOSURE |
            c.XCB_EVENT_MASK_POINTER_MOTION |
            c. XCB_EVENT_MASK_STRUCTURE_NOTIFY;

        // { background color, events to capture }
        const value_list = [_]c_int{ @intCast(state.screen.black_pixel), event_values };

        // Making the window
        _ = c.xcb_create_window(
            state.connection,
            c.XCB_COPY_FROM_PARENT, // depth
            state.window,
            state.screen.root, // assign window to screen root
            @intCast(x),
            @intCast(y),
            @intCast(width),
            @intCast(height),
            0, // no border
            c.XCB_WINDOW_CLASS_INPUT_OUTPUT, // we want both input and output
            state.screen.root_visual,
            event_mask,
            @ptrCast(&value_list),
        );
        
        // Change window name
        _ = c.xcb_change_property(
            state.connection,
            c.XCB_PROP_MODE_REPLACE,
            state.window,
            c.XCB_ATOM_WM_NAME,
            c.XCB_ATOM_STRING,
            8,
            @intCast(std.mem.span(app_name).len),
            app_name,
        );

        // Telling the server to send an event when the window is being destroyed
        const wm_delete_cookie: c.xcb_intern_atom_cookie_t = c.xcb_intern_atom(
            state.connection,
            0,
            @intCast("WM_DELETE_WINDOW".len),
            "WM_DELETE_WINDOW",
        );
        const wm_protocols_cookie: c.xcb_intern_atom_cookie_t = c.xcb_intern_atom(
            state.connection,
            0,
            @intCast("WM_PROTOCOLS".len),
            "WM_PROTOCOLS",
        );
        const wm_delete_reply: *c.xcb_intern_atom_reply_t = c.xcb_intern_atom_reply(
            state.connection,
            wm_delete_cookie,
            null,
        );
        const wm_protocols_reply: *c.xcb_intern_atom_reply_t = c.xcb_intern_atom_reply(
            state.connection,
            wm_protocols_cookie,
            null,
        );

        state.wm_delete_win = wm_delete_reply.atom;
        state.wm_protocols = wm_protocols_reply.atom;

        _ = c.xcb_change_property(
            state.connection,
            c.XCB_PROP_MODE_REPLACE,
            state.window,
            wm_protocols_reply.atom,
            4,
            32,
            1,
            &wm_delete_reply.atom,
        );

        _ = c.xcb_map_window(state.connection, state.window);

        const stream_result = c.xcb_flush(state.connection);
        if (stream_result <= 0) {
            log.err("Error when flushing the X server stream: {d}", .{stream_result});
            return error.XFlushError;
        }

        return state;
    }

    pub fn deinit(self: *LinuxState) void {
        _ = c.XAutoRepeatOn(self.display);
        _ = c.xcb_destroy_window(self.connection, self.window);
    }

    pub fn pollEvents(self: *LinuxState) bool {
        var quit: bool = false;
        var client_message: ?*c.xcb_client_message_event_t = null;
        var maybe_event: ?*c.xcb_generic_event_t = null;
        defer if (maybe_event) |event| std.heap.c_allocator.destroy(event);

        while (true) {
            maybe_event = c.xcb_poll_for_event(self.connection);
            if (maybe_event) |event| {
                switch (event.response_type & ~@as(i32, 0x80)) {
                    c.XCB_KEY_PRESS | c.XCB_KEY_RELEASE => {
                        // TODO keyboard inputs
                    },
                    c.XCB_BUTTON_PRESS | c.XCB_BUTTON_RELEASE => {
                        // TODO mouse inputs
                    },
                    c.XCB_MOTION_NOTIFY => {
                        // TODO mouse movement
                    },
                    c.XCB_CONFIGURE_NOTIFY => {
                        // TODO resizing
                    },

                    c.XCB_CLIENT_MESSAGE => {
                        client_message = @ptrCast(event);
                        if (client_message.?.data.data32[0] == self.wm_delete_win) {
                            // Window close event
                            quit = true;
                        }
                    },

                    else => log.warn(
                        "Unhandled window event 0x{X}", 
                        .{event.response_type & ~@as(i32, 0x80)}
                    ),
                }
            } else {
                break;
            }
        }

        return !quit;
    }
};
