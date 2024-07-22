#pragma once

#include "gen/PlaydateAPI.hpp"
#include "NativeResource.hpp"

namespace cranked {

    template<numeric_type T>
    struct Vector2 {
        template<numeric_type S>
        [[nodiscard]] Vector2<S> as() const {
            return Vector2<S>((S) x, (S) y);
        }

        template<numeric_type S>
        [[nodiscard]] auto operator+(const Vector2<S> other) const {
            return Vector2{ x + other.x, y + other.y };
        }

        template<numeric_type S>
        [[nodiscard]] auto operator-(const Vector2<S> other) const {
            return Vector2{ x - other.x, y - other.y };
        }

        template<numeric_type S>
        [[nodiscard]] auto operator*(S scale) const {
            return Vector2{ x * scale, y * scale };
        }

        template<numeric_type S>
        [[nodiscard]] auto operator/(S scale) const {
            return Vector2{ x / scale, y / scale };
        }

        template<numeric_type S>
        Vector2 &operator-=(const Vector2<S> other) {
            return *this = *this - other;
        }

        template<numeric_type S>
        Vector2 &operator+=(const Vector2<S> other) {
            return *this = *this + other;
        }

        template<numeric_type S>
        Vector2 &operator*=(S scale) {
            return *this = *this * scale;
        }

        template<numeric_type S>
        Vector2 &operator/=(S scale) {
            return *this = *this / scale;
        }

        [[nodiscard]] Vector2 operator-() const {
            return { -x, -y };
        }

        template<numeric_type S>
        [[nodiscard]] bool operator==(const Vector2<S> other) const {
            return x == other.x and y == other.y;
        }

        [[nodiscard]] operator bool() const { // NOLINT(*-explicit-constructor)
            return x != T(0) or y != T(0);
        }

        [[nodiscard]] CollisionPoint_32 asCollisionPoint() const {
            return { (float)x, (float)y };
        }

        [[nodiscard]] CollisionVector_32 asCollisionVector() const {
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
        [[nodiscard]] auto operator+(const Vector2<S> offset) const {
            return Rectangle{ pos.x + offset.x, pos.y + offset.y, size.x, size.y };
        }

        template<numeric_type S>
        [[nodiscard]] auto operator-(const Vector2<S> offset) const {
            return Rectangle{ pos.x - offset.x, pos.y - offset.y, size.x, size.y };
        }

        template<numeric_type S>
        Rectangle &operator+=(const Vector2<S> offset) {
            return *this = *this + offset;
        }

        template<numeric_type S>
        Rectangle &operator-=(const Vector2<S> offset) {
            return *this = *this - offset;
        }

        template<numeric_type S>
        [[nodiscard]] bool operator==(const Rectangle<S> other) const {
            return pos == other.pos and size == other.size;
        }

        template<numeric_type S>
        [[nodiscard]] Rectangle intersection(const Rectangle<S> &other) const {
            using A = decltype(T() + S());
            Rectangle<A> rect{};
            if (other.pos.x > pos.x) {
                rect.pos.x = (A)other.pos.x;
                rect.size.x = max((A)0, min((A)other.size.x, (A)size.x - ((A)pos.x - (A)other.pos.x)));
            } else {
                rect.pos.x = (A)pos.x;
                rect.size.x = max((A)0, min((A)size.x, (A)other.size.x - ((A)other.pos.x - (A)pos.x)));
            }
            if (other.pos.y > pos.y) {
                rect.pos.y = (A)other.pos.y;
                rect.size.y = max((A)0, min((A)other.size.y, (A)size.y - ((A)pos.y - (A)other.pos.y)));
            } else {
                rect.pos.y = (A)pos.y;
                rect.size.y = max((A)0, min((A)size.y, (A)other.size.y - ((A)other.pos.y - (A)pos.y)));
            }
            return rect;
        }

        [[nodiscard]] operator bool() const { // NOLINT(*-explicit-constructor)
            return size;
        }

        template<numeric_type S>
        [[nodiscard]] Vector2<T> nearestCorner(Vector2<S> point) const {
            return { nearestValue(pos.x, pos.x + size.x, point.x), nearestValue(pos.y, pos.y + size.y, point.y) };
        }

        template<numeric_type S>
        [[nodiscard]] Vector2<T> rotateAbout(float angle, Vector2<S> pivot) const {
            auto p = *this - pivot;
            float c = cosf(angle), s = sinf(angle);
            p = { p.x * c - p.y * s, p.x * s + p.y * c };
            return p + pivot;
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
