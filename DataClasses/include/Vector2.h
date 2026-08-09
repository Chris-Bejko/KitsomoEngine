#pragma once
#include <iostream>
#include <cmath>
#include <limits>
#include <string>

// Base vector with common utilities
template<typename T, typename Derived>
struct VectorBase
{
    // Magnitude
    inline T magnitude() const
    {
        T sum = 0;
        const T* data = reinterpret_cast<const T*>(this);
        for (size_t i = 0; i < Derived::Size; i++)
            sum += data[i] * data[i];
        return std::sqrt(sum);
    }

    inline T magnitudeSq() const
    {
        T sum = 0;
        const T* data = reinterpret_cast<const T*>(this);
        for (size_t i = 0; i < Derived::Size; i++)
            sum += data[i] * data[i];
        return sum;
    }

    inline Derived normalized() const
    {
        T mag = magnitude();
        if (mag <= 0) return Derived();
        Derived result = *static_cast<const Derived*>(this);
        T* data = reinterpret_cast<T*>(&result);
        for (size_t i = 0; i < Derived::Size; i++)
            data[i] /= mag;
        return result;
    }

    inline Derived& zero()
    {
        T* data = reinterpret_cast<T*>(this);
        for (size_t i = 0; i < Derived::Size; i++)
            data[i] = 0;
        return *static_cast<Derived*>(this);
    }

    inline Derived& ones()
    {
        T* data = reinterpret_cast<T*>(this);
        for (size_t i = 0; i < Derived::Size; i++)
            data[i] = 1;
        return *static_cast<Derived*>(this);
    }

    // Dot product
    inline T dot(const Derived& other) const
    {
        T sum = 0;
        const T* a = reinterpret_cast<const T*>(this);
        const T* b = reinterpret_cast<const T*>(&other);
        for (size_t i = 0; i < Derived::Size; i++)
            sum += a[i] * b[i];
        return sum;
    }

    // Distance
    inline T distance(const Derived& other) const
    {
        return ((*static_cast<const Derived*>(this)) - other).magnitude();
    }

    // Lerp
    inline Derived lerp(const Derived& other, T t) const
    {
        return (*static_cast<const Derived*>(this)) * (1 - t) + other * t;
    }

    // Comparison
    inline bool operator==(const Derived& other) const
    {
        const T* a = reinterpret_cast<const T*>(this);
        const T* b = reinterpret_cast<const T*>(&other);
        for (size_t i = 0; i < Derived::Size; i++)
            if (a[i] != b[i]) return false;
        return true;
    }

    inline bool operator!=(const Derived& other) const
    {
        return !(*this == other);
    }

    // String
    inline std::string toString() const
    {
        std::string result = "(";
        const T* data = reinterpret_cast<const T*>(this);
        for (size_t i = 0; i < Derived::Size; i++)
        {
            result += std::to_string(data[i]);
            if (i < Derived::Size - 1) result += ", ";
        }
        return result + ")";
    }
};

// Vector2
template<typename T>
struct Vector2 : VectorBase<T, Vector2<T>>
{
    static constexpr size_t Size = 2;
    T x, y;

    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}

    inline Vector2<T> operator+(const Vector2<T>& v) const { return {x + v.x, y + v.y}; }
    inline Vector2<T> operator-(const Vector2<T>& v) const { return {x - v.x, y - v.y}; }
    inline Vector2<T> operator*(const T s)            const { return {x * s,   y * s};   }
    inline Vector2<T> operator/(const T d)            const { return d != 0 ? Vector2<T>{x/d, y/d} : Vector2<T>(); }

    inline friend Vector2<T>& operator+=(Vector2<T>& a, const Vector2<T>& b) { a.x+=b.x; a.y+=b.y; return a; }
    inline friend Vector2<T>& operator-=(Vector2<T>& a, const Vector2<T>& b) { a.x-=b.x; a.y-=b.y; return a; }
    inline friend Vector2<T>& operator*=(Vector2<T>& a, T s)                 { a.x*=s;   a.y*=s;   return a; }

    inline friend std::ostream& operator<<(std::ostream& s, const Vector2<T>& v)
    { s << "(" << v.x << "," << v.y << ")"; return s; }
};

// Vector3
template<typename T>
struct Vector3 : VectorBase<T, Vector3<T>>
{
    static constexpr size_t Size = 3;
    T x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

    inline Vector3<T> operator+(const Vector3<T>& v) const { return {x+v.x, y+v.y, z+v.z}; }
    inline Vector3<T> operator-(const Vector3<T>& v) const { return {x-v.x, y-v.y, z-v.z}; }
    inline Vector3<T> operator*(const T s)            const { return {x*s,   y*s,   z*s};   }
    inline Vector3<T> operator/(const T d)            const { return d != 0 ? Vector3<T>{x/d,y/d,z/d} : Vector3<T>(); }

    inline friend Vector3<T>& operator+=(Vector3<T>& a, const Vector3<T>& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
    inline friend Vector3<T>& operator-=(Vector3<T>& a, const Vector3<T>& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }
    inline friend Vector3<T>& operator*=(Vector3<T>& a, T s)                 { a.x*=s;   a.y*=s;   a.z*=s;   return a; }

    // Cross product (3D only)
    inline Vector3<T> cross(const Vector3<T>& v) const
    {
        return { y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x };
    }

    inline friend std::ostream& operator<<(std::ostream& s, const Vector3<T>& v)
    { s << "(" << v.x << "," << v.y << "," << v.z << ")"; return s; }
};

// Vector4
template<typename T>
struct Vector4 : VectorBase<T, Vector4<T>>
{
    static constexpr size_t Size = 4;
    T x, y, z, w;

    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    inline Vector4<T> operator+(const Vector4<T>& v) const { return {x+v.x,y+v.y,z+v.z,w+v.w}; }
    inline Vector4<T> operator-(const Vector4<T>& v) const { return {x-v.x,y-v.y,z-v.z,w-v.w}; }
    inline Vector4<T> operator*(const T s)            const { return {x*s,  y*s,  z*s,  w*s};   }
    inline Vector4<T> operator/(const T d)            const { return d != 0 ? Vector4<T>{x/d,y/d,z/d,w/d} : Vector4<T>(); }

    inline friend Vector4<T>& operator+=(Vector4<T>& a, const Vector4<T>& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; a.w+=b.w; return a; }
    inline friend Vector4<T>& operator-=(Vector4<T>& a, const Vector4<T>& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; a.w-=b.w; return a; }

    inline friend std::ostream& operator<<(std::ostream& s, const Vector4<T>& v)
    { s << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")"; return s; }
};

// Aliases
using Vector2I  = Vector2<int>;
using Vector2U  = Vector2<unsigned int>;
using Vector2F  = Vector2<float>;
using Vector2LF = Vector2<double>;

using Vector3I  = Vector3<int>;
using Vector3F  = Vector3<float>;
using Vector3LF = Vector3<double>;

using Vector4I  = Vector4<int>;
using Vector4F  = Vector4<float>;
using Vector4LF = Vector4<double>;