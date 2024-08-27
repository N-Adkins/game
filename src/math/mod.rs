mod vec2;
mod vec3;

pub use num_traits::Num;
use num_traits::{AsPrimitive, NumCast};

#[allow(unused)]
pub use vec2::Vec2;

#[allow(unused)]
pub use vec3::Vec3;

pub trait Number: Num + Default + Copy + Clone + AsPrimitive<f32> + NumCast {}
impl<T: Num + Default + Copy + Clone + AsPrimitive<f32> + NumCast> Number for T {}
