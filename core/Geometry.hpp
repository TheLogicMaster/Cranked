#pragma once

#include "gen/PlaydateAPI.hpp"

namespace cranked {

    template<numeric_type T>
    struct Vector2 {
        template<numeric_type S>
        [[nodiscard]] Vector2<S> as() const {
            return Vector2<S>((S) x, (S) y);
        }

        [[nodiscard]] Vector2<float> normalized() const {
            float len = length();
            return { (float)x / len, (float)y / len };
        }

        [[nodiscard]] float length() const {
            return sqrtf((float)x * (float)x + (float)y * (float)y);
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

        [[nodiscard]] bool isInvalid() const {
            return isnanf((float)x) or isnanf((float)y);
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
        float m11, m12, tx;
        float m21, m22, ty;

        [[nodiscard]] constexpr Transform invert() const {
            float det = m11 * m22 - m12 * m21;
            return {
                m22 / det, -m12 / det, (m12 * ty - m22 * tx) / det,
                -m21 / det, m11 / det, (m21 * tx - m11 * ty) / det
            };
        }

        template<numeric_type S>
        [[nodiscard]] constexpr Vector2<S> operator*(Vector2<S> point) const {
            return Vec2{
                m11 * (float)point.x + m12 * (float)point.y + tx,
                m21 * (float)point.x + m22 * (float)point.y + ty,
            }.as<S>();
        }

        // Todo: Probably want pre-multiplication rather than post
        [[nodiscard]] constexpr Transform operator*(const Transform &other) const {
            return {
                m11 * other.m11 + m12 * other.m21,
                m11 * other.m12 + m12 * other.m22,
                m11 * other.tx + m12 * other.ty + tx,
                m21 * other.m11 + m22 * other.m21,
                m21 * other.m12 + m22 * other.m22,
                m21 * other.tx + m22 * other.ty + ty
            };
        }

        static constexpr Transform identity() {
            return {
                1, 0, 0,
                0, 1, 0
            };
        }

        static constexpr Transform scale(float xScale, float yScale) {
            return {
                xScale, 0, 0,
                0, yScale, 0
            };
        }

        static constexpr Transform rotate(float angle) {
            return {
                cosf(angle), -sinf(angle), 0,
                sinf(angle), cosf(angle), 0
            };
        }

        template<numeric_type S>
        static constexpr Transform translate(Vector2<S> vec) {
            return {
                1, 0, (float)vec.x,
                0, 1, (float)vec.y
            };
        }
    };

}
