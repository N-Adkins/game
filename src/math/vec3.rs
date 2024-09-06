use std::ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Sub, SubAssign};

use super::Number;

#[derive(Debug, Default, Clone, Copy, PartialEq, Eq)]
pub struct Vec3<T: Number = f32> {
    x: T,
    y: T,
    z: T,
}

#[allow(unused)]
impl<T: Number> Vec3<T> {
    pub fn new(x: T, y: T, z: T) -> Self {
        Self { x, y, z }
    }

    pub fn zero() -> Self {
        Self {
            x: T::zero(),
            y: T::zero(),
            z: T::zero(),
        }
    }

    pub fn dot(lhs: Self, rhs: Self) -> f32 {
        lhs.x.as_() * rhs.x.as_() + lhs.y.as_() * rhs.y.as_() + lhs.z.as_() * rhs.z.as_()
    }

    pub fn magnitude(&self) -> f32 {
        (self.x.as_() * self.x.as_() + self.y.as_() * self.y.as_() + self.z.as_() * self.z.as_())
            .sqrt()
    }

    pub fn unit(&self) -> Self {
        let mag = T::from(self.magnitude()).unwrap();
        *self / mag
    }
}

impl<T: Number> Add for Vec3<T> {
    type Output = Vec3<T>;

    fn add(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
            z: self.z + rhs.z,
        }
    }
}

impl<T: Number> AddAssign for Vec3<T> {
    fn add_assign(&mut self, rhs: Self) {
        self.x = self.x + rhs.x;
        self.y = self.y + rhs.y;
        self.z = self.z + rhs.z;
    }
}

impl<T: Number> Sub for Vec3<T> {
    type Output = Vec3<T>;

    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
            z: self.z - rhs.z,
        }
    }
}

impl<T: Number> SubAssign for Vec3<T> {
    fn sub_assign(&mut self, rhs: Self) {
        self.x = self.x - rhs.x;
        self.y = self.y - rhs.y;
        self.z = self.z - rhs.z;
    }
}

impl<T: Number> Mul<T> for Vec3<T> {
    type Output = Vec3<T>;

    fn mul(self, rhs: T) -> Self::Output {
        Self {
            x: self.x * rhs,
            y: self.y * rhs,
            z: self.z * rhs,
        }
    }
}

impl<T: Number> MulAssign<T> for Vec3<T> {
    fn mul_assign(&mut self, rhs: T) {
        self.x = self.x * rhs;
        self.y = self.y * rhs;
        self.z = self.z * rhs;
    }
}

impl<T: Number> Div<T> for Vec3<T> {
    type Output = Vec3<T>;

    fn div(self, rhs: T) -> Self::Output {
        Self {
            x: self.x / rhs,
            y: self.y / rhs,
            z: self.z / rhs,
        }
    }
}

impl<T: Number> DivAssign<T> for Vec3<T> {
    fn div_assign(&mut self, rhs: T) {
        self.x = self.x / rhs;
        self.y = self.y / rhs;
        self.z = self.z / rhs;
    }
}

#[cfg(test)]
mod tests {
    // Easier to test on integers
    type Vec3 = super::Vec3<i32>;

    #[test]
    fn addition() {
        let mut lhs = Vec3::new(10, 7, 5);
        let rhs = Vec3::new(71, 23, 8);
        let result = lhs + rhs;
        lhs += rhs;
        assert_eq!(result, Vec3::new(81, 30, 13));
        assert_eq!(lhs, result);
    }

    #[test]
    fn subtraction() {
        let mut lhs = Vec3::new(10, 7, 5);
        let rhs = Vec3::new(71, 23, 8);
        let result = lhs - rhs;
        lhs -= rhs;
        assert_eq!(result, Vec3::new(-61, -16, -3));
        assert_eq!(lhs, result);
    }

    #[test]
    fn multiplication() {
        let mut lhs = Vec3::new(10, 7, 5);
        let rhs: i32 = 12;
        let result = lhs * rhs;
        lhs *= rhs;
        assert_eq!(result, Vec3::new(120, 84, 60));
        assert_eq!(lhs, result);
    }

    #[test]
    fn division() {
        let mut lhs = Vec3::new(72, 144, 36);
        let rhs: i32 = 12;
        let result = lhs / rhs;
        lhs /= rhs;
        assert_eq!(result, Vec3::new(6, 12, 3));
        assert_eq!(lhs, result);
    }

    #[test]
    fn magnitude() {
        let vec = Vec3::new(4, 3, 0);
        assert_eq!(vec.magnitude(), 5.0);
    }

    #[test]
    fn dot() {
        let lhs = Vec3::new(3, 4, 1);
        let rhs = Vec3::new(5, 6, 8);
        assert_eq!(Vec3::dot(lhs, rhs), 47.0);
    }
}
