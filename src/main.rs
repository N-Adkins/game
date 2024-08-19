mod window;
use window::backend::Backend;

fn main() {
    tracing_subscriber::fmt::init();
    let mut backend = window::glfw::GlfwBackend::init();
    let _window = backend.create_window();
}
