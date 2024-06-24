#pragma once

#include "gen/PlaydateAPI.hpp"
#include "HeapAllocator.hpp"
#include "NativeResource.hpp"

namespace cranked {

    template<numeric_type T>
    struct Vector2 {
        template<numeric_type S>
        [[nodiscard]] Vector2<S> as() const {
            return Vector2<S>((S) x, (S) y);
        }

        template<numeric_type S>
        auto operator+(const Vector2<S> other) const {
            return Vector2{ x + other.x, y + other.y };
        }

        template<numeric_type S>
        auto operator-(const Vector2<S> other) const {
            return Vector2{ x - other.x, y - other.y };
        }

        template<numeric_type S>
        auto operator*(S scale) const {
            return Vector2{ x * scale, y * scale };
        }

        template<numeric_type S>
        auto operator/(S scale) const {
            return Vector2{ x / scale, y / scale };
        }

        template<numeric_type S>
        Vector2<T> &operator-=(const Vector2<S> other) {
            return *this = *this - other;
        }

        template<numeric_type S>
        Vector2<T> &operator+=(const Vector2<S> other) {
            return *this = *this + other;
        }

        template<numeric_type S>
        Vector2<T> &operator*=(S scale) {
            return *this = *this * scale;
        }

        template<numeric_type S>
        Vector2<T> &operator/=(S scale) {
            return *this = *this / scale;
        }

        Vector2 operator-() const {
            return { -x, -y };
        }

        template<numeric_type S>
        bool operator==(const Vector2<S> other) const {
            return x == other.x and y == other.y;
        }

        operator bool() { // NOLINT(*-explicit-constructor)
            return x != T(0) or y != T(0);
        }

        CollisionPoint_32 asCollisionPoint() {
            return { (float)x, (float)y };
        }

        CollisionVector_32 asCollisionVector() {
            return { (int32)x, (int32)y };
        }

        T x, y;
    };

    typedef Vector2<int32> IntVec2;
    typedef Vector2<float> Vec2;

    template<numeric_type T>
    struct LineSegment {
        Vector2<T> start, end;
    };

    typedef LineSegment<int32> IntLineSeg;
    typedef LineSegment<float> LineSeg;

    template<numeric_type T>
    struct Rectangle {
        template<numeric_type S>
        [[nodiscard]] Rectangle<S> as() const {
            return Rectangle<S>(pos.template as<S>(), size.template as<S>());
        }

        template<numeric_type S>
        [[nodiscard]] bool contains(Vector2<S> point) const {
            return point.x >= pos.x and point.x < pos.x + size.x and point.y >= pos.y and point.y < pos.y + size.y;
        }

        template<numeric_type S>
        auto operator+(const Vector2<S> offset) const {
            return Rectangle{ pos.x + offset.x, pos.y + offset.y, size.x, size.y };
        }

        template<numeric_type S>
        auto operator-(const Vector2<S> offset) const {
            return Rectangle{ pos.x - offset.x, pos.y - offset.y, size.x, size.y };
        }

        template<numeric_type S>
        Rectangle<T> &operator+=(const Vector2<S> offset) {
            return *this = *this + offset;
        }

        template<numeric_type S>
        Rectangle<T> &operator-=(const Vector2<S> offset) {
            return *this = *this - offset;
        }

        template<numeric_type S>
        bool operator==(const Rectangle<S> other) const {
            return pos == other.pos and size == other.size;
        }

        template<numeric_type S>
        Rectangle intersection(const Rectangle<S> &other) {
            Rectangle rect{};
            if (other.pos.x < pos.x) {
                rect.pos.x = other.pos.x;
                rect.size.x = max(0.0f, min(other.size.x, size.x - (pos.x - other.pos.x)));
            } else {
                rect.pos.x = pos.x;
                rect.size.x = max(0.0f, min(size.x, other.size.x - (other.pos.x - pos.x)));
            }
            if (other.pos.y < pos.y) {
                rect.pos.y = other.pos.y;
                rect.size.y = max(0.0f, min(other.size.y, size.y - (pos.y - other.pos.y)));
            } else {
                rect.pos.y = pos.y;
                rect.size.y = max(0.0f, min(size.y, other.size.y - (other.pos.y - pos.y)));
            }
            return rect;
        }

        operator bool() { // NOLINT(*-explicit-constructor)
            return size;
        }

        template<numeric_type S>
        Vector2<T> nearestCorner(Vector2<S> point) {
            return { nearestValue(pos.x, pos.x + size.x, point.x), nearestValue(pos.y, pos.y + size.y, point.y) };
        }

        template<numeric_type S>
        bool contains(Vector2<S> point) {
            return point.x >= pos.x and point.y >= pos.y and point.x <= pos.x + size.x and point.y <= pos.y + size.y;
        }

        Vector2<T> pos;
        Vector2<T> size;
    };

    typedef Rectangle<int32> IntRect;
    typedef Rectangle<float> Rect;

    static IntRect toRect(LCDRect_32 rect) {
        return { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
    }

    static LCDRect_32 fromRect(IntRect rect) {
        return { rect.pos.x, rect.pos.y, rect.pos.x + rect.size.x, rect.pos.y + rect.size.y };
    }

    static Rect toRect(PDRect_32 rect) {
        return { rect.x, rect.y, rect.width, rect.height };
    }

    static PDRect_32 fromRect(Rect rect) {
        return { rect.pos.x, rect.pos.y, rect.size.x, rect.size.y };
    }

    struct Transform {
        // Todo

        float m11, m12, m21, m22, tx, ty;
    };

}
