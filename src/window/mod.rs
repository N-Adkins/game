pub mod backend;
pub mod glfw;

pub trait Window {
    fn get_size(&self) -> (u32, u32);
}
