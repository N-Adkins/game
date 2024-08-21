pub mod renderer_backends;
pub mod window_backends;

pub trait Window: std::fmt::Debug {
    fn set_callbacks(&mut self);
    fn get_size(&self) -> (u32, u32);
    fn should_close(&self) -> bool;
    fn set_should_close(&mut self, value: bool);
    fn get_events(&mut self) -> Vec<Event>;
    fn get_requested_extensions(&mut self) -> Vec<String>;
}

#[derive(Debug, Clone, Copy)]
pub enum Event {
    KeyPressed(KeyCode),
}

#[derive(Debug, Clone, Copy)]
pub enum KeyCode {
    Escape,
}

pub trait Renderer: std::fmt::Debug {}

#[derive(Debug)]
pub struct GraphicsContext {
    window: Box<dyn Window>,
    renderer: Box<dyn Renderer>,
}
