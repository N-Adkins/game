#[derive(Debug, Default, Clone, Copy, PartialEq)]
pub struct Vec2 {
    x: f32,
    y: f32,
}

impl Vec2 {
    pub fn zero() -> Self {
        Self { x: 0.0, y: 0.0 }
    }
}
