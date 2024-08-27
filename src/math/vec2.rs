use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Sub, SubAssign};

use super::Number;

#[derive(Debug, Default, Clone, Copy, PartialEq)]
pub struct Vec2<T: Number = f32> {
    x: T,
    y: T,
}

#[allow(unused)]
impl<T: Number> Vec2<T> {
    pub fn new(x: T, y: T) -> Self {
        Self { x, y }
    }

    pub fn zero() -> Self {
        Self {
            x: T::zero(),
            y: T::zero(),
        }
    }

    pub fn dot(lhs: Self, rhs: Self) -> f32 {
        lhs.x.as_() * rhs.x.as_() + lhs.y.as_() * rhs.y.as_()
    }

    pub fn magnitude(&self) -> f32 {
        (self.x.as_() + self.y.as_()).sqrt()
    }

    pub fn unit(&self) -> Self {
        let mag = T::from(self.magnitude()).unwrap();
        *self / mag
    }
}

impl<T: Number> Add for Vec2<T> {
    type Output = Vec2<T>;

    fn add(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
        }
    }
}

impl<T: Number> AddAssign for Vec2<T> {
    fn add_assign(&mut self, rhs: Self) {
        self.x = self.x + rhs.x;
        self.y = self.y + rhs.y;
    }
}

impl<T: Number> Sub for Vec2<T> {
    type Output = Vec2<T>;

    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
        }
    }
}

impl<T: Number> SubAssign for Vec2<T> {
    fn sub_assign(&mut self, rhs: Self) {
        self.x = self.x - rhs.x;
        self.y = self.y - rhs.y;
    }
}

impl<T: Number> Mul<T> for Vec2<T> {
    type Output = Vec2<T>;

    fn mul(self, rhs: T) -> Self::Output {
        Self {
            x: self.x * rhs,
            y: self.y * rhs,
        }
    }
}

impl<T: Number> MulAssign<T> for Vec2<T> {
    fn mul_assign(&mut self, rhs: T) {
        self.x = self.x * rhs;
        self.y = self.y * rhs;
    }
}

impl<T: Number> Div<T> for Vec2<T> {
    type Output = Vec2<T>;

    fn div(self, rhs: T) -> Self::Output {
        Self {
            x: self.x / rhs,
            y: self.y / rhs,
        }
    }
}

impl<T: Number> DivAssign<T> for Vec2<T> {
    fn div_assign(&mut self, rhs: T) {
        self.x = self.x / rhs;
        self.y = self.y / rhs;
    }
}
