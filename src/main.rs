use anyhow::Result;
use graphics::{window_backends::glfw, Event, KeyCode, Window};

mod graphics;

fn main() -> Result<()> {
    tracing_subscriber::fmt::init();

    let glfw_window = glfw::GLFWWindow::new()?;
    let mut window: Box<dyn Window> = Box::new(glfw_window);
    window.set_callbacks();

    while !window.should_close() {
        let events = window.get_events();
        for event in events {
            match event {
                Event::KeyPressed(KeyCode::Escape) => window.set_should_close(true),
            }
        }
    }

    Ok(())
}
