use super::Window;

pub trait Backend {
    fn init() -> impl Backend;
    fn create_window(&mut self) -> impl Window;
}
