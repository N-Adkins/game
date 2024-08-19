pub mod backend;
pub mod glfw;

pub trait Window {
    fn get_size(&self) -> (u32, u32);
    fn should_close(&self) -> bool;
    fn close(&mut self);
    fn get_events(&mut self) -> Vec<Event>;
}

#[derive(Debug)]
pub enum Event {
    KeyPressed(KeyCode),
}

#[derive(Debug)]
pub enum KeyCode {
    Escape,
}
