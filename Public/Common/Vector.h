#pragma once


#include "../RSDKTypes.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

inline constexpr float RSDK_PI = 3.14159265358979323846f;
inline constexpr float RSDK_DEG_TO_RAD = RSDK_PI / 180.0f;
inline constexpr float RSDK_RAD_TO_DEG = 180.0f / RSDK_PI;

[[nodiscard]] constexpr float ToRadian(const float degrees) noexcept
{
    return degrees * RSDK_DEG_TO_RAD;
}

[[nodiscard]] constexpr float ToDegree(const float radians) noexcept
{
    return radians * RSDK_RAD_TO_DEG;
}

class Vec2
{
public:
    float x{};
    float y{};

    constexpr Vec2() noexcept = default;
    constexpr Vec2(const float xValue, const float yValue) noexcept : x(xValue), y(yValue) {}
    constexpr Vec2(const RSDK_Vec2& value) noexcept : x(value.x), y(value.y) {}
    [[nodiscard]] constexpr operator RSDK_Vec2() const noexcept { return {x, y}; }

    constexpr void Set(const float xValue, const float yValue) noexcept { x = xValue; y = yValue; }
    constexpr void Zero() noexcept { x = 0.0f; y = 0.0f; }

    [[nodiscard]] constexpr float operator[](const std::size_t index) const noexcept { return index == 0u ? x : y; }
    constexpr float& operator[](const std::size_t index) noexcept { return index == 0u ? x : y; }

    [[nodiscard]] constexpr Vec2 operator-() const noexcept { return {-x, -y}; }
    [[nodiscard]] constexpr float operator*(const Vec2& other) const noexcept { return DotProduct(other); }
    [[nodiscard]] constexpr Vec2 operator*(const float scalar) const noexcept { return {x * scalar, y * scalar}; }
    [[nodiscard]] constexpr Vec2 operator/(const float scalar) const noexcept { return {x / scalar, y / scalar}; }
    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const noexcept { return {x + other.x, y + other.y}; }
    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const noexcept { return {x - other.x, y - other.y}; }

    constexpr Vec2& operator+=(const Vec2& other) noexcept { x += other.x; y += other.y; return *this; }
    constexpr Vec2& operator-=(const Vec2& other) noexcept { x -= other.x; y -= other.y; return *this; }
    constexpr Vec2& operator*=(const float scalar) noexcept { x *= scalar; y *= scalar; return *this; }
    constexpr Vec2& operator/=(const float scalar) noexcept { x /= scalar; y /= scalar; return *this; }
    constexpr Vec2& operator/=(const Vec2& other) noexcept { x /= other.x; y /= other.y; return *this; }

    friend constexpr Vec2 operator*(const float scalar, const Vec2& value) noexcept { return value * scalar; }

    [[nodiscard]] constexpr bool operator==(const Vec2& other) const noexcept { return x == other.x && y == other.y; }
    [[nodiscard]] constexpr bool operator!=(const Vec2& other) const noexcept { return !(*this == other); }
    [[nodiscard]] constexpr bool Compare(const Vec2& other) const noexcept { return *this == other; }

    [[nodiscard]] constexpr float LengthSqr() const noexcept { return x * x + y * y; }
    [[nodiscard]] float Length() const noexcept { return std::sqrt(LengthSqr()); }
    [[nodiscard]] constexpr float DotProduct(const Vec2& other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] constexpr Vec2 Perpendicular() const noexcept { return {y, -x}; }
    [[nodiscard]] bool IsFinite() const noexcept { return std::isfinite(x) && std::isfinite(y); }
    [[nodiscard]] bool IsValid(const float epsilon = std::numeric_limits<float>::epsilon()) const noexcept { return IsFinite() && LengthSqr() > epsilon * epsilon; }

    Vec2& VectorNormalize(const float epsilon = std::numeric_limits<float>::epsilon()) noexcept
    {
        const float length = Length();
        if (length > epsilon) { *this /= length; } else { Zero(); }
        return *this;
    }

    [[nodiscard]] Vec2 Normalized(const float epsilon = std::numeric_limits<float>::epsilon()) const noexcept
    {
        Vec2 result = *this;
        result.VectorNormalize(epsilon);
        return result;
    }

    [[nodiscard]] Vec2 Rotated(const float angleRadians) const noexcept
    {
        const float cosine = std::cos(angleRadians);
        const float sine = std::sin(angleRadians);
        return {x * cosine - y * sine, y * cosine + x * sine};
    }

    [[nodiscard]] float DistanceTo(const Vec2& other) const noexcept { return (other - *this).Length(); }
    [[nodiscard]] constexpr float DistanceToSqr(const Vec2& other) const noexcept { return (other - *this).LengthSqr(); }

    [[nodiscard]] float Polar() const noexcept
    {
        float degrees = ToDegree(std::atan2(y, x));
        if (degrees < 0.0f) { degrees += 360.0f; }
        return degrees;
    }

    [[nodiscard]] float AngleBetween(const Vec2& other) const noexcept
    {
        const float denominator = Length() * other.Length();
        if (denominator <= std::numeric_limits<float>::epsilon()) { return 0.0f; }
        const float cosine = std::clamp(DotProduct(other) / denominator, -1.0f, 1.0f);
        return ToDegree(std::acos(cosine));
    }

    [[nodiscard]] Vec2 Extend(const Vec2& other, const float length) const noexcept
    {
        return *this + (other - *this).Normalized() * length;
    }

    [[nodiscard]] bool Intersects(
        const Vec2& segment1End,
        const Vec2& segment2Start,
        const Vec2& segment2End,
        Vec2* const intersectionOut = nullptr,
        const float epsilon = 1.0e-6f) const noexcept
    {
        const Vec2 p = *this;
        const Vec2 r = segment1End - p;
        const Vec2 q = segment2Start;
        const Vec2 s = segment2End - q;
        const auto cross = [](const Vec2& a, const Vec2& b) noexcept { return a.x * b.y - a.y * b.x; };

        const float rCrossS = cross(r, s);
        const Vec2 qMinusP = q - p;
        const float qMinusPCrossR = cross(qMinusP, r);

        if (std::fabs(rCrossS) <= epsilon)
        {
            if (std::fabs(qMinusPCrossR) > epsilon) { return false; }
            const float rLengthSqr = r.LengthSqr();
            if (rLengthSqr <= epsilon * epsilon)
            {
                if (p.DistanceToSqr(q) <= epsilon * epsilon) { if (intersectionOut) *intersectionOut = p; return true; }
                return false;
            }
            float t0 = qMinusP.DotProduct(r) / rLengthSqr;
            float t1 = t0 + s.DotProduct(r) / rLengthSqr;
            if (t0 > t1) std::swap(t0, t1);
            const float overlapStart = std::max(0.0f, t0);
            const float overlapEnd = std::min(1.0f, t1);
            if (overlapStart > overlapEnd + epsilon) { return false; }
            if (intersectionOut) *intersectionOut = p + r * std::clamp(overlapStart, 0.0f, 1.0f);
            return true;
        }

        const float t = cross(qMinusP, s) / rCrossS;
        const float u = cross(qMinusP, r) / rCrossS;
        if (t < -epsilon || t > 1.0f + epsilon || u < -epsilon || u > 1.0f + epsilon) { return false; }
        if (intersectionOut) *intersectionOut = p + r * std::clamp(t, 0.0f, 1.0f);
        return true;
    }
};

class Vec3
{
public:
    float x{};
    float y{};
    float z{};

    constexpr Vec3() noexcept = default;
    constexpr Vec3(const float xValue, const float yValue, const float zValue) noexcept : x(xValue), y(yValue), z(zValue) {}
    constexpr Vec3(const RSDK_Vec3& value) noexcept : x(value.x), y(value.y), z(value.z) {}
    [[nodiscard]] constexpr operator RSDK_Vec3() const noexcept { return {x, y, z}; }

    constexpr void Set(const float xValue, const float yValue, const float zValue) noexcept { x = xValue; y = yValue; z = zValue; }
    constexpr void Zero() noexcept { x = y = z = 0.0f; }

    [[nodiscard]] constexpr float operator[](const std::size_t index) const noexcept { return index == 0u ? x : (index == 1u ? y : z); }
    constexpr float& operator[](const std::size_t index) noexcept { return index == 0u ? x : (index == 1u ? y : z); }

    [[nodiscard]] constexpr Vec3 operator-() const noexcept { return {-x, -y, -z}; }
    [[nodiscard]] constexpr float operator*(const Vec3& other) const noexcept { return DotProduct(other); }
    [[nodiscard]] constexpr Vec3 operator*(const float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }
    [[nodiscard]] constexpr Vec3 operator/(const float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }
    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept { return {x + other.x, y + other.y, z + other.z}; }
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept { return {x - other.x, y - other.y, z - other.z}; }

    constexpr Vec3& operator+=(const Vec3& other) noexcept { x += other.x; y += other.y; z += other.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& other) noexcept { x -= other.x; y -= other.y; z -= other.z; return *this; }
    constexpr Vec3& operator*=(const float scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; return *this; }
    constexpr Vec3& operator/=(const float scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; return *this; }
    constexpr Vec3& operator/=(const Vec3& other) noexcept { x /= other.x; y /= other.y; z /= other.z; return *this; }

    friend constexpr Vec3 operator*(const float scalar, const Vec3& value) noexcept { return value * scalar; }

    [[nodiscard]] constexpr bool operator==(const Vec3& other) const noexcept { return x == other.x && y == other.y && z == other.z; }
    [[nodiscard]] constexpr bool operator!=(const Vec3& other) const noexcept { return !(*this == other); }
    [[nodiscard]] constexpr bool Compare(const Vec3& other) const noexcept { return *this == other; }

    [[nodiscard]] constexpr float LengthSqr() const noexcept { return x * x + y * y + z * z; }
    [[nodiscard]] constexpr float Length2DSqr() const noexcept { return x * x + z * z; }
    [[nodiscard]] float Length() const noexcept { return std::sqrt(LengthSqr()); }
    [[nodiscard]] float Length2D() const noexcept { return std::sqrt(Length2DSqr()); }
    [[nodiscard]] constexpr float DotProduct(const Vec3& other) const noexcept { return x * other.x + y * other.y + z * other.z; }
    [[nodiscard]] constexpr Vec3 CrossProduct(const Vec3& other) const noexcept { return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x}; }
    [[nodiscard]] constexpr Vec2 To2D() const noexcept { return {x, z}; }
    [[nodiscard]] constexpr Vec3 Perpendicular() const noexcept { return {z, y, -x}; }
    [[nodiscard]] bool IsFinite() const noexcept { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
    [[nodiscard]] bool IsValid(const float epsilon = std::numeric_limits<float>::epsilon()) const noexcept { return IsFinite() && LengthSqr() > epsilon * epsilon; }

    Vec3& VectorNormalize(const float epsilon = std::numeric_limits<float>::epsilon()) noexcept
    {
        const float length = Length();
        if (length > epsilon) { *this /= length; } else { Zero(); }
        return *this;
    }

    [[nodiscard]] Vec3 Normalized(const float epsilon = std::numeric_limits<float>::epsilon()) const noexcept
    {
        Vec3 result = *this;
        result.VectorNormalize(epsilon);
        return result;
    }

    [[nodiscard]] Vec3 Rotated(const float angleRadians) const noexcept
    {
        const float cosine = std::cos(angleRadians);
        const float sine = std::sin(angleRadians);
        return {x * cosine - z * sine, y, z * cosine + x * sine};
    }

    [[nodiscard]] float Distance2D(const Vec3& other) const noexcept { return (other - *this).Length2D(); }
    [[nodiscard]] float DistanceTo(const Vec3& other) const noexcept { return (other - *this).Length(); }
    [[nodiscard]] constexpr float DistanceToSqr(const Vec3& other) const noexcept { return (other - *this).LengthSqr(); }
    [[nodiscard]] Vec3 Extend(const Vec3& other, const float length) const noexcept { return *this + (other - *this).Normalized() * length; }
};

class Vec4
{
public:
    float x{};
    float y{};
    float z{};
    float w{};

    constexpr Vec4() noexcept = default;
    constexpr Vec4(const float xValue, const float yValue, const float zValue, const float wValue) noexcept : x(xValue), y(yValue), z(zValue), w(wValue) {}

    constexpr void Set(const float xValue, const float yValue, const float zValue, const float wValue) noexcept { x = xValue; y = yValue; z = zValue; w = wValue; }
    constexpr void Zero() noexcept { x = y = z = w = 0.0f; }
    constexpr void Negate() noexcept { x = -x; y = -y; z = -z; w = -w; }

    [[nodiscard]] constexpr float operator[](const std::size_t index) const noexcept { return index == 0u ? x : (index == 1u ? y : (index == 2u ? z : w)); }
    constexpr float& operator[](const std::size_t index) noexcept { return index == 0u ? x : (index == 1u ? y : (index == 2u ? z : w)); }

    [[nodiscard]] constexpr Vec4 operator-() const noexcept { return {-x, -y, -z, -w}; }
    [[nodiscard]] constexpr float operator*(const Vec4& other) const noexcept { return DotProduct(other); }
    [[nodiscard]] constexpr Vec4 operator*(const float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
    [[nodiscard]] constexpr Vec4 operator/(const float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar, w / scalar}; }
    [[nodiscard]] constexpr Vec4 operator+(const Vec4& other) const noexcept { return {x + other.x, y + other.y, z + other.z, w + other.w}; }
    [[nodiscard]] constexpr Vec4 operator-(const Vec4& other) const noexcept { return {x - other.x, y - other.y, z - other.z, w - other.w}; }

    constexpr Vec4& operator+=(const Vec4& other) noexcept { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    constexpr Vec4& operator-=(const Vec4& other) noexcept { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    constexpr Vec4& operator*=(const float scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    constexpr Vec4& operator/=(const float scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }
    constexpr Vec4& operator/=(const Vec4& other) noexcept { x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; }

    friend constexpr Vec4 operator*(const float scalar, const Vec4& value) noexcept { return value * scalar; }

    [[nodiscard]] constexpr bool operator==(const Vec4& other) const noexcept { return x == other.x && y == other.y && z == other.z && w == other.w; }
    [[nodiscard]] constexpr bool operator!=(const Vec4& other) const noexcept { return !(*this == other); }
    [[nodiscard]] constexpr bool Compare(const Vec4& other) const noexcept { return *this == other; }

    [[nodiscard]] constexpr float LengthSqr() const noexcept { return x * x + y * y + z * z + w * w; }
    [[nodiscard]] float Length() const noexcept { return std::sqrt(LengthSqr()); }
    [[nodiscard]] constexpr float DotProduct(const Vec4& other) const noexcept { return x * other.x + y * other.y + z * other.z + w * other.w; }
    [[nodiscard]] constexpr float DotProduct(const Vec3& other) const noexcept { return x * other.x + y * other.y + z * other.z; }
    [[nodiscard]] bool IsFinite() const noexcept { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w); }

    Vec4& VectorNormalize(const float epsilon = std::numeric_limits<float>::epsilon()) noexcept
    {
        const float length = Length();
        if (length > epsilon) { *this /= length; } else { Zero(); }
        return *this;
    }

    [[nodiscard]] Vec4 Normalized(const float epsilon = std::numeric_limits<float>::epsilon()) const noexcept
    {
        Vec4 result = *this;
        result.VectorNormalize(epsilon);
        return result;
    }
};

static_assert(std::is_standard_layout_v<Vec2> && std::is_trivially_copyable_v<Vec2>);
static_assert(std::is_standard_layout_v<Vec3> && std::is_trivially_copyable_v<Vec3>);
static_assert(std::is_standard_layout_v<Vec4> && std::is_trivially_copyable_v<Vec4>);
static_assert(sizeof(Vec2) == sizeof(float) * 2u);
static_assert(sizeof(Vec3) == sizeof(float) * 3u);
static_assert(sizeof(Vec4) == sizeof(float) * 4u);

namespace rsdk
{
    using ::Vec2;
    using ::Vec3;
    using ::Vec4;
}
