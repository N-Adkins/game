#pragma once

namespace sol {
    class state;
}

namespace Engine {
    
class Vec2 {
public:
    Vec2(float x, float y = 0)
        : x(x), y(y) {}

    float getX() const;
    float getY() const;
    void setX(float x);
    void setY(float y);
    
    static float dot(const Vec2& lhs, const Vec2& rhs);
    float magnitude() const;
    Vec2 unit() const;

    Vec2 operator+(const Vec2& rhs) const;
    void operator+=(const Vec2& rhs);
    Vec2 operator-(const Vec2& rhs) const;
    void operator-=(const Vec2& rhs);
    Vec2 operator*(float rhs) const;
    void operator*=(float rhs);
    Vec2 operator/(float rhs) const;
    void operator/=(float rhs);
    Vec2 operator-() const; // unary negation
    bool operator==(const Vec2& rhs) const;

    static void registerLua(sol::state& lua);

private:
    union {
        struct {
            float x;
            float y;
        };
        float raw[2];
    };
};

} // namespace Engine
