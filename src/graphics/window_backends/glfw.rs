#![allow(unused)]

use std::ffi::{CStr};
use std::ptr::null_mut;

use libc::{c_char, c_int, c_void};
use tracing::instrument;
use tracing::{error, trace};

use crate::graphics::{Event, KeyCode, Window};

#[repr(C)]
struct GLFWwindow {
    _data: [u8; 0],
    _marker: core::marker::PhantomData<(*mut u8, core::marker::PhantomPinned)>,
}

#[repr(C)]
struct GLFWmonitor {
    _data: [u8; 0],
    _marker: core::marker::PhantomData<(*mut u8, core::marker::PhantomPinned)>,
}

const GLFW_TRUE: c_int = 1;
const GLFW_FALSE: c_int = 0;

const GLFW_PRESS: c_int = 1;
const GLFW_RELEASE: c_int = 0;
const GLFW_REPEAT: c_int = 2;

const GLFW_KEY_UNKNOWN: c_int = -1;
const GLFW_KEY_SPACE: c_int = 32;
const GLFW_KEY_APOSTROPHE: c_int = 39;
const GLFW_KEY_COMMA: c_int = 44;
const GLFW_KEY_MINUS: c_int = 45;
const GLFW_KEY_PERIOD: c_int = 46;
const GLFW_KEY_SLASH: c_int = 47;
const GLFW_KEY_0: c_int = 48;
const GLFW_KEY_1: c_int = 49;
const GLFW_KEY_2: c_int = 50;
const GLFW_KEY_3: c_int = 51;
const GLFW_KEY_4: c_int = 52;
const GLFW_KEY_5: c_int = 53;
const GLFW_KEY_6: c_int = 54;
const GLFW_KEY_7: c_int = 55;
const GLFW_KEY_8: c_int = 56;
const GLFW_KEY_9: c_int = 57;
const GLFW_KEY_SEMICOLON: c_int = 59;
const GLFW_KEY_EQUAL: c_int = 61;
const GLFW_KEY_A: c_int = 65;
const GLFW_KEY_B: c_int = 66;
const GLFW_KEY_C: c_int = 67;
const GLFW_KEY_D: c_int = 68;
const GLFW_KEY_E: c_int = 69;
const GLFW_KEY_F: c_int = 70;
const GLFW_KEY_G: c_int = 71;
const GLFW_KEY_H: c_int = 72;
const GLFW_KEY_I: c_int = 73;
const GLFW_KEY_J: c_int = 74;
const GLFW_KEY_K: c_int = 75;
const GLFW_KEY_L: c_int = 76;
const GLFW_KEY_M: c_int = 77;
const GLFW_KEY_N: c_int = 78;
const GLFW_KEY_O: c_int = 79;
const GLFW_KEY_P: c_int = 80;
const GLFW_KEY_Q: c_int = 81;
const GLFW_KEY_R: c_int = 82;
const GLFW_KEY_S: c_int = 83;
const GLFW_KEY_T: c_int = 84;
const GLFW_KEY_U: c_int = 85;
const GLFW_KEY_V: c_int = 86;
const GLFW_KEY_W: c_int = 87;
const GLFW_KEY_X: c_int = 88;
const GLFW_KEY_Y: c_int = 89;
const GLFW_KEY_Z: c_int = 90;
const GLFW_KEY_LEFT_BRACKET: c_int = 91;
const GLFW_KEY_BACKSLASH: c_int = 92;
const GLFW_KEY_RIGHT_BRACKET: c_int = 93;
const GLFW_KEY_GRAVE_ACCENT: c_int = 96;
const GLFW_KEY_WORLD_1: c_int = 161;
const GLFW_KEY_WORLD_2: c_int = 162;
const GLFW_KEY_ESCAPE: c_int = 256;
const GLFW_KEY_ENTER: c_int = 257;
const GLFW_KEY_TAB: c_int = 258;
const GLFW_KEY_BACKSPACE: c_int = 259;
const GLFW_KEY_INSERT: c_int = 260;
const GLFW_KEY_DELETE: c_int = 261;
const GLFW_KEY_RIGHT: c_int = 262;
const GLFW_KEY_LEFT: c_int = 263;
const GLFW_KEY_DOWN: c_int = 264;
const GLFW_KEY_UP: c_int = 265;
const GLFW_KEY_PAGE_UP: c_int = 266;
const GLFW_KEY_PAGE_DOWN: c_int = 267;
const GLFW_KEY_HOME: c_int = 268;
const GLFW_KEY_END: c_int = 269;
const GLFW_KEY_CAPS_LOCK: c_int = 280;
const GLFW_KEY_SCROLL_LOCK: c_int = 281;
const GLFW_KEY_NUM_LOCK: c_int = 282;
const GLFW_KEY_PRINT_SCREEN: c_int = 283;
const GLFW_KEY_PAUSE: c_int = 284;
const GLFW_KEY_F1: c_int = 290;
const GLFW_KEY_F2: c_int = 291;
const GLFW_KEY_F3: c_int = 292;
const GLFW_KEY_F4: c_int = 293;
const GLFW_KEY_F5: c_int = 294;
const GLFW_KEY_F6: c_int = 295;
const GLFW_KEY_F7: c_int = 296;
const GLFW_KEY_F8: c_int = 297;
const GLFW_KEY_F9: c_int = 298;
const GLFW_KEY_F10: c_int = 299;
const GLFW_KEY_F11: c_int = 300;
const GLFW_KEY_F12: c_int = 301;
const GLFW_KEY_F13: c_int = 302;
const GLFW_KEY_F14: c_int = 303;
const GLFW_KEY_F15: c_int = 304;
const GLFW_KEY_F16: c_int = 305;
const GLFW_KEY_F17: c_int = 306;
const GLFW_KEY_F18: c_int = 307;
const GLFW_KEY_F19: c_int = 308;
const GLFW_KEY_F20: c_int = 309;
const GLFW_KEY_F21: c_int = 310;
const GLFW_KEY_F22: c_int = 311;
const GLFW_KEY_F23: c_int = 312;
const GLFW_KEY_F24: c_int = 313;
const GLFW_KEY_F25: c_int = 314;
const GLFW_KEY_KP_0: c_int = 320;
const GLFW_KEY_KP_1: c_int = 321;
const GLFW_KEY_KP_2: c_int = 322;
const GLFW_KEY_KP_3: c_int = 323;
const GLFW_KEY_KP_4: c_int = 324;
const GLFW_KEY_KP_5: c_int = 325;
const GLFW_KEY_KP_6: c_int = 326;
const GLFW_KEY_KP_7: c_int = 327;
const GLFW_KEY_KP_8: c_int = 328;
const GLFW_KEY_KP_9: c_int = 329;
const GLFW_KEY_KP_DECIMAL: c_int = 330;
const GLFW_KEY_KP_DIVIDE: c_int = 331;
const GLFW_KEY_KP_MULTIPLY: c_int = 332;
const GLFW_KEY_KP_SUBTRACT: c_int = 333;
const GLFW_KEY_KP_ADD: c_int = 334;
const GLFW_KEY_KP_ENTER: c_int = 335;
const GLFW_KEY_KP_EQUAL: c_int = 336;
const GLFW_KEY_LEFT_SHIFT: c_int = 340;
const GLFW_KEY_LEFT_CONTROL: c_int = 341;
const GLFW_KEY_LEFT_ALT: c_int = 342;
const GLFW_KEY_LEFT_SUPER: c_int = 343;
const GLFW_KEY_RIGHT_SHIFT: c_int = 344;
const GLFW_KEY_RIGHT_CONTROL: c_int = 345;
const GLFW_KEY_RIGHT_ALT: c_int = 346;
const GLFW_KEY_RIGHT_SUPER: c_int = 347;
const GLFW_KEY_MENU: c_int = 348;
const GLFW_KEY_LAST: c_int = GLFW_KEY_MENU;

#[link(name = "glfw")]
extern "C" {
    fn glfwInit() -> c_int;
    fn glfwTerminate();
    fn glfwSetErrorCallback(callback: extern "C" fn(error_code: c_int, description: *const c_char));
    fn glfwSetWindowUserPointer(window: *mut GLFWwindow, pointer: *mut c_void);
    fn glfwGetWindowUserPointer(window: *mut GLFWwindow) -> *mut c_void;
    fn glfwCreateWindow(
        width: c_int,
        height: c_int,
        title: *const c_char,
        monitor: *mut GLFWmonitor,
        share: *mut GLFWwindow,
    ) -> *mut GLFWwindow;
    fn glfwDestroyWindow(window: *mut GLFWwindow);
    fn glfwGetWindowSize(window: *mut GLFWwindow, width: *mut c_int, height: *mut c_int);
    fn glfwWindowShouldClose(window: *mut GLFWwindow) -> c_int;
    fn glfwSetWindowShouldClose(window: *mut GLFWwindow, value: c_int);
    fn glfwMakeContextCurrent(window: *mut GLFWwindow);
    fn glfwPollEvents();
    fn glfwSetKeyCallback(
        window: *mut GLFWwindow,
        callback: extern "C" fn(
            window: *mut GLFWwindow,
            key: c_int,
            scancode: c_int,
            action: c_int,
            mods: c_int,
        ),
    );
}

#[derive(Debug)]
struct GLFWContext {
    events: Vec<Event>,
}

impl GLFWContext {
    #[instrument]
    pub fn new() -> Self {
        trace!("Attempting to initialize GLFW context");
        let result: c_int = unsafe { glfwInit() };
        if result != GLFW_TRUE {
            error!("Failed to initialize GLFW context");
            panic!("Failed to initialize GLFW context");
        }
        unsafe { glfwSetErrorCallback(Self::error_callback) };
        Self { events: Vec::new() }
    }

    #[instrument]
    extern "C" fn error_callback(error_code: c_int, description: *const c_char) {
        if description.is_null() {
            return;
        }
        let c_str = unsafe {
            CStr::from_ptr(description)
                .to_str()
                .expect("GLFW UTF8 Error")
        };
        error!("GLFW Error {error_code}: {c_str}");
    }

    #[instrument]
    extern "C" fn key_callback(
        window: *mut GLFWwindow,
        key: c_int,
        scancode: c_int,
        action: c_int,
        mods: c_int,
    ) {
        let context = unsafe {
            let ptr = glfwGetWindowUserPointer(window) as *mut GLFWContext;
            if ptr.is_null() {
                panic!("GLFW user pointer is null, forgot to set it?");
            }
            &mut *ptr
        };
        if action == GLFW_PRESS {
            let keycode: Option<KeyCode> = match key {
                GLFW_KEY_ESCAPE => Some(KeyCode::Escape),
                _ => None,
            };
            if let Some(key) = keycode {
                context.events.push(Event::KeyPressed(key));
            }
        }
    }
}

impl Drop for GLFWContext {
    #[instrument]
    fn drop(&mut self) {
        trace!("Terminating GLFW context");
        unsafe { glfwTerminate() }
    }
}

#[derive(Debug)]
pub struct GLFWWindow {
    handle: *mut GLFWwindow,
    context: GLFWContext,
}

impl GLFWWindow {
    #[instrument]
    pub fn new() -> Self {
        trace!("Attempting to create GLFW window");
        let context = GLFWContext::new();
        let handle = unsafe {
            let handle = glfwCreateWindow(
                1280,
                720,
                "Game\0".as_ptr() as *const i8,
                null_mut(),
                null_mut(),
            );
            if handle.is_null() {
                panic!("Failed to create GLFW window");
            }
            handle
        };
        unsafe { glfwMakeContextCurrent(handle) };
        Self { handle, context }
    }
}

impl Drop for GLFWWindow {
    fn drop(&mut self) {
        trace!("Destroying GLFW window");
        unsafe { glfwDestroyWindow(self.handle) }
    }
}

impl Window for GLFWWindow {
    fn set_callbacks(&mut self) {
        unsafe {
            glfwSetWindowUserPointer(
                self.handle,
                (&mut self.context as *mut GLFWContext) as *mut c_void,
            );
            glfwSetKeyCallback(self.handle, GLFWContext::key_callback);
        }
    }

    fn get_size(&self) -> (u32, u32) {
        let mut width: c_int = 0;
        let mut height: c_int = 0;
        unsafe { glfwGetWindowSize(self.handle, &mut width, &mut height) };
        (width as u32, height as u32)
    }

    fn get_events(&mut self) -> Vec<Event> {
        let events = self.context.events.clone();
        self.context.events.clear();
        unsafe { glfwPollEvents() };
        events
    }

    fn should_close(&self) -> bool {
        let value = unsafe { glfwWindowShouldClose(self.handle) };
        value != 0
    }

    fn set_should_close(&mut self, value: bool) {
        unsafe { glfwSetWindowShouldClose(self.handle, value as i32) }
    }
}
