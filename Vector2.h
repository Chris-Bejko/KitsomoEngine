#pragma once
#include <iostream>

template <typename T>
struct Vector2
{
    T x, y;

    Vector2()
    {
        this->x = 0;
        this->y = 0;
    }

    Vector2(T x, T y)
    {
        this->x = x;
        this->y = y;
    }

    inline Vector2<T> operator+(const Vector2<T>& v2) const
    {
        return Vector2<T>(x + v2.x, y + v2.y);
    }

    inline friend Vector2<T>& operator+=(Vector2<T>& v1, const Vector2<T>& v2)
    {
        v1.x += v2.x;
        v1.y += v2.y;
        return v1;
    }

    inline Vector2<T> operator-(const Vector2<T>& v2)
    {
        return Vector2<T>(x - v2.x, y - v2.y);
    }

    inline friend Vector2<T> operator-=(Vector2<T>& v1, const Vector2<T>& v2)
    {
        v1.x -= v2.x;
        v1.y -= v2.y;
        return v1;
    }

    inline Vector2<T> operator*(const T scalar) const
    {
        return Vector2<T>(x * scalar, y * scalar);
    }

    inline Vector2<T> operator/(const T d) const
    {
        return (d != 0 ? Vector2<T>(x / d, y / d) : Vector2<T>());
    }

    inline Vector2<T>& zero()
    {
        this->x = 0;
        this->y = 0;
        return *this;
    }

    inline Vector2<T>& ones()
    {
        this->x = 1;
        this->y = 1;
        return *this;
    }

    inline friend std::ostream& operator <<(std::ostream& stream, const Vector2<T>& v)
    {
        stream << "(" << v.x << "," << v.y << ")";
        return stream;
    }
};

using Vector2I = Vector2<int>;
using Vector2U = Vector2<unsigned int>;
using Vector2F = Vector2<float>;
using Vector2LF = Vector2<double>;