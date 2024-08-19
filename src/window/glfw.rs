use tracing::{error, instrument, trace};

use super::{backend::Backend, Window};

#[derive(Debug)]
pub struct GlfwBackend {
    glfw: glfw::Glfw,
}

impl Backend for GlfwBackend {
    #[instrument]
    fn init() -> Self {
        trace!("Attempting to initialize GLFW backend");

        let glfw = match glfw::init(move |_err, message| {
            error!("GLFW error: {message}");
        }) {
            Ok(glfw) => glfw,
            Err(err) => {
                error!("GLFW error: {err}");
                panic!("Failed to initialize GLFW backend");
            }
        };

        trace!("Initialized GLFW backend");

        return Self {
            glfw,
        }
    }
    
    fn create_window(&mut self) -> impl Window {
        GlfwWindow::new(self)
    }
}

#[derive(Debug)]
struct GlfwWindow {
    handle: glfw::PWindow,
    event_receiver: glfw::GlfwReceiver<(f64, glfw::WindowEvent)>
}

impl GlfwWindow {
    #[instrument]
    fn new(backend: &mut GlfwBackend) -> Self {
        trace!("Attempting to create GLFW window");

        let (handle, event_receiver) = match backend.glfw.create_window(1280, 720, "Game", glfw::WindowMode::Windowed) {
            Some(handle) => handle,
            None => {
                error!("Failed to create GLFW window");
                panic!("Failed to create GLFW window");
            }
        };

        trace!("Created GLFW window");

        Self {
            handle,
            event_receiver,
        }
    }
}

impl Window for GlfwWindow {
    fn get_size(&self) -> (u32, u32) {
        let (x, y) = self.handle.get_size();
        (x as u32, y as u32)
    }
}
