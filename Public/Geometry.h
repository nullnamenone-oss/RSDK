#pragma once

#include "Common/Vector.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace rsdk::geometry
{
    inline constexpr float Epsilon = 1.0e-5f;

    [[nodiscard]] inline float DistanceSquared(const Vec2& a, const Vec2& b) noexcept
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    [[nodiscard]] inline float Distance(const Vec2& a, const Vec2& b) noexcept
    {
        return std::sqrt(DistanceSquared(a, b));
    }

    [[nodiscard]] inline float DistanceSquared(const Vec3& a, const Vec3& b) noexcept
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    [[nodiscard]] inline float Distance(const Vec3& a, const Vec3& b) noexcept
    {
        return std::sqrt(DistanceSquared(a, b));
    }

    [[nodiscard]] inline float Dot(const Vec2& a, const Vec2& b) noexcept
    {
        return a.x * b.x + a.y * b.y;
    }

    [[nodiscard]] inline float Dot(const Vec3& a, const Vec3& b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    [[nodiscard]] inline float Cross(const Vec2& a, const Vec2& b) noexcept
    {
        return a.x * b.y - a.y * b.x;
    }

    [[nodiscard]] inline Vec3 Cross(const Vec3& a, const Vec3& b) noexcept
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    // SOURCE L#: LeagueSharp.Common/Geometry.cs::ProjectionInfo.
    // RSDK uses Vec2 for the game X/Z plane.
    struct ProjectionInfo final
    {
        bool IsOnSegment{};
        Vec2 SegmentPoint{};
        Vec2 LinePoint{};
    };

    // SOURCE L#: LeagueSharp.Common/Geometry.cs::ProjectOn(Vector2, Vector2, Vector2).
    // The unclamped projection is LinePoint; SegmentPoint is clamped to [start, end].
    [[nodiscard]] inline ProjectionInfo ProjectOn(
        const Vec2& point, const Vec2& segmentStart, const Vec2& segmentEnd) noexcept
    {
        const Vec2 segment = segmentEnd - segmentStart;
        const float lengthSquared = Dot(segment, segment);

        // RSDK-CHANGE: prevent division by zero for a zero-length segment.
        // There is no meaningful side or finite line in this case.
        if (lengthSquared == 0.0f)
            return {false, segmentStart, segmentStart};

        const float rL = Dot(point - segmentStart, segment) / lengthSquared;
        const Vec2 pointLine = segmentStart + segment * rL;
        const float rS = std::clamp(rL, 0.0f, 1.0f);
        const bool isOnSegment = rS == rL;
        const Vec2 pointSegment = isOnSegment ? pointLine : segmentStart + segment * rS;
        return {isOnSegment, pointSegment, pointLine};
    }

    // SOURCE L#: ezEvade/Helpers/Position.cs::isLeftOfLineSegment.
    // Tests the directed infinite line, NOT whether the point lies on the segment.
    [[nodiscard]] inline bool isLeftOfLineSegment(
        const Vec2& pos, const Vec2& start, const Vec2& end) noexcept
    {
        return Cross(end - start, pos - start) > 0.0f;
    }

    // RSDK public naming convention; the lower-case function above retains L# spelling.
    [[nodiscard]] inline bool IsLeftOfLineSegment(
        const Vec2& pos, const Vec2& start, const Vec2& end) noexcept
    {
        return isLeftOfLineSegment(pos, start, end);
    }

    [[nodiscard]] inline Vec2 ClosestPointOnSegment(
        const Vec2& point,
        const Vec2& segmentStart,
        const Vec2& segmentEnd) noexcept
    {
        const Vec2 segment{segmentEnd.x - segmentStart.x, segmentEnd.y - segmentStart.y};
        const float lengthSquared = Dot(segment, segment);
        if (lengthSquared <= Epsilon)
            return segmentStart;

        const Vec2 offset{point.x - segmentStart.x, point.y - segmentStart.y};
        const float t = std::clamp(Dot(offset, segment) / lengthSquared, 0.0f, 1.0f);
        return {segmentStart.x + segment.x * t, segmentStart.y + segment.y * t};
    }

    [[nodiscard]] inline Vec3 ClosestPointOnSegment(
        const Vec3& point,
        const Vec3& segmentStart,
        const Vec3& segmentEnd) noexcept
    {
        const Vec3 segment{
            segmentEnd.x - segmentStart.x,
            segmentEnd.y - segmentStart.y,
            segmentEnd.z - segmentStart.z};
        const float lengthSquared = Dot(segment, segment);
        if (lengthSquared <= Epsilon)
            return segmentStart;

        const Vec3 offset{
            point.x - segmentStart.x,
            point.y - segmentStart.y,
            point.z - segmentStart.z};
        const float t = std::clamp(Dot(offset, segment) / lengthSquared, 0.0f, 1.0f);
        return {
            segmentStart.x + segment.x * t,
            segmentStart.y + segment.y * t,
            segmentStart.z + segment.z * t
        };
    }

    [[nodiscard]] inline float DistanceSquaredToSegment(
        const Vec2& point,
        const Vec2& segmentStart,
        const Vec2& segmentEnd) noexcept
    {
        return DistanceSquared(point, ClosestPointOnSegment(point, segmentStart, segmentEnd));
    }

    [[nodiscard]] inline float DistanceSquaredToSegment(
        const Vec3& point,
        const Vec3& segmentStart,
        const Vec3& segmentEnd) noexcept
    {
        return DistanceSquared(point, ClosestPointOnSegment(point, segmentStart, segmentEnd));
    }

    [[nodiscard]] inline float DistanceToSegment(
        const Vec2& point,
        const Vec2& segmentStart,
        const Vec2& segmentEnd) noexcept
    {
        return std::sqrt(DistanceSquaredToSegment(point, segmentStart, segmentEnd));
    }

    [[nodiscard]] inline float DistanceToSegment(
        const Vec3& point,
        const Vec3& segmentStart,
        const Vec3& segmentEnd) noexcept
    {
        return std::sqrt(DistanceSquaredToSegment(point, segmentStart, segmentEnd));
    }

    [[nodiscard]] inline bool SegmentIntersection(
        const Vec2& a0,
        const Vec2& a1,
        const Vec2& b0,
        const Vec2& b1,
        Vec2& intersection) noexcept
    {
        const Vec2 r{a1.x - a0.x, a1.y - a0.y};
        const Vec2 s{b1.x - b0.x, b1.y - b0.y};
        const float denominator = Cross(r, s);
        if (std::fabs(denominator) <= Epsilon)
            return false;

        const Vec2 difference{b0.x - a0.x, b0.y - a0.y};
        const float t = Cross(difference, s) / denominator;
        const float u = Cross(difference, r) / denominator;
        if (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f)
            return false;

        intersection = {a0.x + r.x * t, a0.y + r.y * t};
        return true;
    }

    [[nodiscard]] inline bool IsInsideCircle(
        const Vec2& point,
        const Vec2& center,
        const float radius) noexcept
    {
        return radius >= 0.0f && DistanceSquared(point, center) <= radius * radius;
    }

    [[nodiscard]] inline bool IsInsideCircle(
        const Vec3& point,
        const Vec3& center,
        const float radius) noexcept
    {
        return radius >= 0.0f && DistanceSquared(point, center) <= radius * radius;
    }

    struct Circle2D final
    {
        Vec2 Center{};
        float Radius{};
        bool Valid{};
    };

    [[nodiscard]] inline Circle2D CircleFromTwoPoints(const Vec2& a, const Vec2& b) noexcept
    {
        const Vec2 center{(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        const double dx = static_cast<double>(center.x) - static_cast<double>(a.x);
        const double dy = static_cast<double>(center.y) - static_cast<double>(a.y);
        return {center, static_cast<float>(std::sqrt(dx * dx + dy * dy)), true};
    }

    [[nodiscard]] inline Circle2D CircleFromThreePoints(
        const Vec2& a,
        const Vec2& b,
        const Vec2& c) noexcept
    {
        const double ax = static_cast<double>(a.x);
        const double ay = static_cast<double>(a.y);
        const double bx = static_cast<double>(b.x);
        const double by = static_cast<double>(b.y);
        const double cx = static_cast<double>(c.x);
        const double cy = static_cast<double>(c.y);
        const double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
        if (std::fabs(d) <= static_cast<double>(Epsilon))
            return {};

        const double a2 = ax * ax + ay * ay;
        const double b2 = bx * bx + by * by;
        const double c2 = cx * cx + cy * cy;
        const double centerX = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
        const double centerY = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
        const double dx = centerX - ax;
        const double dy = centerY - ay;
        const Vec2 center{static_cast<float>(centerX), static_cast<float>(centerY)};
        return {center, static_cast<float>(std::sqrt(dx * dx + dy * dy)), true};
    }

    [[nodiscard]] inline bool Encloses(const Circle2D& circle, const std::vector<Vec2>& points) noexcept
    {
        if (!circle.Valid)
            return false;

        const double radius = static_cast<double>(circle.Radius);
        const double scale = std::max({1.0, std::fabs(static_cast<double>(circle.Center.x)), std::fabs(static_cast<double>(circle.Center.y)), std::fabs(radius)});
        const double tolerance = std::max(1.0e-3, scale * static_cast<double>(std::numeric_limits<float>::epsilon()) * 8.0);
        const double expandedRadius = radius + tolerance;
        const double radiusSquared = expandedRadius * expandedRadius;
        for (const Vec2& point : points)
        {
            const double dx = static_cast<double>(point.x) - static_cast<double>(circle.Center.x);
            const double dy = static_cast<double>(point.y) - static_cast<double>(circle.Center.y);
            if (dx * dx + dy * dy > radiusSquared)
                return false;
        }

        return true;
    }

    // LeagueSharp.Common.Geometry.CircleCircleIntersection equivalent.
    // Returns unique intersection points: 0, 1 for a tangent, or 2.
    [[nodiscard]] inline std::vector<Vec2> CircleCircleIntersections(
        const Vec2& center1,
        const float radius1,
        const Vec2& center2,
        const float radius2)
    {
        std::vector<Vec2> result{};
        if (radius1 < 0.0f || radius2 < 0.0f)
            return result;

        const float distance = Distance(center1, center2);
        if (distance <= Epsilon || distance > radius1 + radius2 || distance < std::fabs(radius1 - radius2))
            return result;

        const float along = (radius1 * radius1 - radius2 * radius2 + distance * distance) / (2.0f * distance);
        const float heightSquared = radius1 * radius1 - along * along;
        if (heightSquared < -Epsilon)
            return result;

        const float height = std::sqrt(std::max(0.0f, heightSquared));
        const Vec2 direction = (center2 - center1) / distance;
        const Vec2 base = center1 + direction * along;
        const Vec2 perpendicular{-direction.y, direction.x};
        result.push_back(base + perpendicular * height);
        if (height > Epsilon)
            result.push_back(base - perpendicular * height);
        return result;
    }

    // LeagueSharp MEC.MakeConvexHull equivalent implemented with the monotonic-chain algorithm.
    [[nodiscard]] inline std::vector<Vec2> ConvexHull(const std::vector<Vec2>& points)
    {
        if (points.size() <= 1u)
            return points;

        std::vector<Vec2> sorted = points;
        std::sort(sorted.begin(), sorted.end(), [](const Vec2& a, const Vec2& b)
        {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        });
        sorted.erase(std::unique(sorted.begin(), sorted.end(), [](const Vec2& a, const Vec2& b)
        {
            return a.x == b.x && a.y == b.y;
        }), sorted.end());

        if (sorted.size() <= 2u)
            return sorted;

        const auto turn = [](const Vec2& a, const Vec2& b, const Vec2& c) noexcept
        {
            return Cross(b - a, c - a);
        };

        std::vector<Vec2> hull{};
        hull.reserve(sorted.size() * 2u);

        for (const Vec2& point : sorted)
        {
            while (hull.size() >= 2u && turn(hull[hull.size() - 2u], hull.back(), point) <= 0.0f)
                hull.pop_back();
            hull.push_back(point);
        }

        const std::size_t lowerSize = hull.size();
        for (std::size_t i = sorted.size() - 1u; i-- > 0u;)
        {
            const Vec2& point = sorted[i];
            while (hull.size() > lowerSize && turn(hull[hull.size() - 2u], hull.back(), point) <= 0.0f)
                hull.pop_back();
            hull.push_back(point);
        }

        if (!hull.empty())
            hull.pop_back();
        return hull;
    }

    /*
     * Exact Minimum Enclosing Circle used by LeagueSharp-style AoE/farm helpers.
     * The minimum circle is defined by either two or three convex-hull points.
     * Candidate circles are validated against the original point set.
     */
    [[nodiscard]] inline Circle2D MinimumEnclosingCircle(const std::vector<Vec2>& points)
    {
        if (points.empty())
            return {};
        if (points.size() == 1u)
            return {points.front(), 0.0f, true};

        const std::vector<Vec2> hull = ConvexHull(points);
        if (hull.empty())
            return {};
        if (hull.size() == 1u)
            return {hull.front(), 0.0f, true};

        Circle2D best{};
        best.Radius = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < hull.size(); ++i)
        {
            for (std::size_t j = i + 1u; j < hull.size(); ++j)
            {
                const Circle2D circle = CircleFromTwoPoints(hull[i], hull[j]);
                if (circle.Radius < best.Radius && Encloses(circle, points))
                    best = circle;
            }
        }

        for (std::size_t i = 0; i < hull.size(); ++i)
        {
            for (std::size_t j = i + 1u; j < hull.size(); ++j)
            {
                for (std::size_t k = j + 1u; k < hull.size(); ++k)
                {
                    const Circle2D circle = CircleFromThreePoints(hull[i], hull[j], hull[k]);
                    if (circle.Valid && circle.Radius < best.Radius && Encloses(circle, points))
                        best = circle;
                }
            }
        }

        if (!best.Valid)
        {
            // Defensive fallback for degenerate/numerically invalid input.
            float maxDistance = -1.0f;
            Vec2 first = points.front();
            Vec2 second = points.back();
            for (std::size_t i = 0; i < points.size(); ++i)
            {
                for (std::size_t j = i + 1u; j < points.size(); ++j)
                {
                    const float distance = DistanceSquared(points[i], points[j]);
                    if (distance > maxDistance)
                    {
                        maxDistance = distance;
                        first = points[i];
                        second = points[j];
                    }
                }
            }
            best = CircleFromTwoPoints(first, second);
        }

        return best;
    }

    struct MovementCollisionResult final
    {
        bool Valid{};
        float Time{};
        Vec2 Position{};
    };

    // LeagueSharp.Common.Geometry.VectorMovementCollision equivalent, expressed as a direct quadratic.
    [[nodiscard]] inline MovementCollisionResult VectorMovementCollision(
        const Vec2& targetStart,
        const Vec2& targetEnd,
        const float targetSpeed,
        const Vec2& missileStart,
        const float missileSpeed,
        const float minTime = 0.0f) noexcept
    {
        MovementCollisionResult result{};
        const Vec2 segment = targetEnd - targetStart;
        const float segmentLength = segment.Length();
        if (segmentLength <= Epsilon)
            return result;

        const Vec2 targetVelocity = segment / segmentLength * targetSpeed;
        if (missileSpeed >= std::numeric_limits<float>::max() * 0.5f)
        {
            result.Valid = true;
            result.Time = minTime;
            result.Position = targetStart + targetVelocity * minTime;
            return result;
        }

        const Vec2 relative = targetStart - missileStart;
        const float a = Dot(targetVelocity, targetVelocity) - missileSpeed * missileSpeed;
        const float b = 2.0f * Dot(relative, targetVelocity);
        const float c = Dot(relative, relative);

        float best = std::numeric_limits<float>::max();
        const auto considerRoot = [&](const float time) noexcept
        {
            if (std::isfinite(time) && time >= minTime - Epsilon && time >= 0.0f && time < best)
                best = time;
        };

        if (std::fabs(a) <= Epsilon)
        {
            if (std::fabs(b) <= Epsilon)
                return result;
            considerRoot(-c / b);
        }
        else
        {
            const float discriminant = b * b - 4.0f * a * c;
            if (discriminant < 0.0f)
                return result;

            const float root = std::sqrt(discriminant);
            considerRoot((-b - root) / (2.0f * a));
            considerRoot((-b + root) / (2.0f * a));
        }

        if (!std::isfinite(best) || best == std::numeric_limits<float>::max())
            return result;

        result.Valid = true;
        result.Time = best;
        result.Position = targetStart + targetVelocity * best;
        return result;
    }

    // Continuous relative-motion circle test inside a bounded time interval.
    [[nodiscard]] inline bool MovingCircleIntersectsInterval(
        const Vec2& relativeAtZero,
        const Vec2& relativeVelocity,
        const float radius,
        const float intervalStart,
        const float intervalEnd) noexcept
    {
        if (intervalEnd < intervalStart)
            return false;

        const float radiusSquared = radius * radius;
        const float a = Dot(relativeVelocity, relativeVelocity);
        const float b = 2.0f * Dot(relativeAtZero, relativeVelocity);
        const float c = Dot(relativeAtZero, relativeAtZero) - radiusSquared;

        if (a <= Epsilon)
            return c <= 0.0f;

        const float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
            return false;

        const float root = std::sqrt(discriminant);
        const float t0 = (-b - root) / (2.0f * a);
        const float t1 = (-b + root) / (2.0f * a);
        return (t0 >= intervalStart - Epsilon && t0 <= intervalEnd + Epsilon)
            || (t1 >= intervalStart - Epsilon && t1 <= intervalEnd + Epsilon);
    }
}
