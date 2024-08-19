mod window;
use window::backend::Backend;
use window::Window;

fn main() {
    tracing_subscriber::fmt::init();
    let mut backend = window::glfw::GlfwBackend::init();
    let mut window = backend.create_window();

    while !window.should_close() {
        let events = window.get_events();
        for event in events {
            match event {
                window::Event::KeyPressed(window::KeyCode::Escape) => window.close(),
            }
        }
    }
}
