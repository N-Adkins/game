use glfw::{Action, Key, WindowEvent};
use tracing::{error, instrument, trace, warn};

use super::{backend::Backend, Event, KeyCode, Window};

#[derive(Debug)]
pub struct GlfwBackend {
    glfw: glfw::Glfw,
}

impl Backend for GlfwBackend {
    #[instrument]
    fn init() -> Self {
        trace!("Attempting to initialize GLFW backend");

        let glfw = match glfw::init(|_err, message| {
            error!("GLFW error: {message}");
        }) {
            Ok(glfw) => glfw,
            Err(err) => {
                error!("GLFW error: {err}");
                panic!("Failed to initialize GLFW backend");
            }
        };

        trace!("Initialized GLFW backend");

        return Self { glfw };
    }

    fn create_window(&mut self) -> impl Window {
        GlfwWindow::new(self)
    }
}

#[derive(Debug)]
struct GlfwWindow {
    handle: glfw::PWindow,
    event_receiver: glfw::GlfwReceiver<(f64, glfw::WindowEvent)>,
}

impl GlfwWindow {
    #[instrument]
    fn new(backend: &mut GlfwBackend) -> Self {
        trace!("Attempting to create GLFW window");

        let (mut handle, event_receiver) =
            match backend
                .glfw
                .create_window(1280, 720, "Game", glfw::WindowMode::Windowed)
            {
                Some(handle) => handle,
                None => {
                    panic!("Failed to create GLFW window");
                }
            };

        trace!("Created GLFW window");

        backend.glfw.make_context_current(Some(&handle));
        handle.set_key_polling(true);

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

    fn should_close(&self) -> bool {
        self.handle.should_close()
    }

    #[instrument]
    fn get_events(&mut self) -> Vec<Event> {
        self.handle.glfw.poll_events();

        let mut events: Vec<Event> = Vec::new();
        for (_, event) in glfw::flush_messages(&self.event_receiver) {
            let event: Event = match event {
                WindowEvent::Key(key, _, Action::Press, _) => match key {
                    Key::Escape => Event::KeyPressed(KeyCode::Escape),
                    _ => {
                        warn!("Unhandled GLFW key pressed event: {key:?}");
                        continue;
                    }
                },
                _ => {
                    warn!("Unhandled GLFW window event: {event:?}");
                    continue;
                }
            };
            events.push(event);
        }

        events
    }

    fn close(&mut self) {
        self.handle.set_should_close(true);
    }
}
