/**
 * @file Vector2D.cpp
 * @brief Implementation of the Vector2D class for the game engine.
 * @details Contains implementations for all member functions declared in Vector2D.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Vector2D.h"

namespace gam300 {

    // Initialize static constants
    const Vector2D Vector2D::ZERO(0.0f, 0.0f);
    const Vector2D Vector2D::ONE(1.0f, 1.0f);
    const Vector2D Vector2D::UNIT_X(1.0f, 0.0f);
    const Vector2D Vector2D::UNIT_Y(0.0f, 1.0f);

    // Default constructor
    Vector2D::Vector2D() : x(0.0f), y(0.0f) {}

    // Constructor with components
    Vector2D::Vector2D(float x, float y) : x(x), y(y) {}

    // Copy constructor
    Vector2D::Vector2D(const Vector2D& other) : x(other.x), y(other.y) {}

    // Assignment operator
    Vector2D& Vector2D::operator=(const Vector2D& other) {
        // Check for self-assignment
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    // Addition
    Vector2D Vector2D::operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    // Subtraction
    Vector2D Vector2D::operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    // Scalar multiplication
    Vector2D Vector2D::operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    // Scalar division
    Vector2D Vector2D::operator/(float scalar) const {
        // Check for division by zero
        if (scalar != 0.0f) {
            float invScalar = 1.0f / scalar;
            return Vector2D(x * invScalar, y * invScalar);
        }
        return *this; // Return original vector on division by zero
    }

    // Compound addition
    Vector2D& Vector2D::operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Compound subtraction
    Vector2D& Vector2D::operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Compound scalar multiplication
    Vector2D& Vector2D::operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    // Compound scalar division
    Vector2D& Vector2D::operator/=(float scalar) {
        // Check for division by zero
        if (scalar != 0.0f) {
            float invScalar = 1.0f / scalar;
            x *= invScalar;
            y *= invScalar;
        }
        return *this;
    }

    // Negation
    Vector2D Vector2D::operator-() const {
        return Vector2D(-x, -y);
    }

    // Equality
    bool Vector2D::operator==(const Vector2D& other) const {
        // Use epsilon comparison for floating-point values
        const float EPSILON = 0.000001f;
        return (std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON);
    }

    // Inequality
    bool Vector2D::operator!=(const Vector2D& other) const {
        return !(*this == other);
    }

    // Magnitude (length)
    float Vector2D::magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    // Squared magnitude
    float Vector2D::magnitudeSquared() const {
        return x * x + y * y;
    }

    // Normalize (return a unit vector)
    Vector2D Vector2D::normalize() const {
        float mag = magnitude();
        if (mag > 0.0f) {
            float invMag = 1.0f / mag;
            return Vector2D(x * invMag, y * invMag);
        }
        return *this; // Return original vector if magnitude is zero
    }

    // Normalize in place
    void Vector2D::normalizeInPlace() {
        float mag = magnitude();
        if (mag > 0.0f) {
            float invMag = 1.0f / mag;
            x *= invMag;
            y *= invMag;
        }
    }

    // Dot product
    float Vector2D::dot(const Vector2D& a, const Vector2D& b) {
        return a.x * b.x + a.y * b.y;
    }

    // 2D cross product (returns scalar)
    float Vector2D::cross(const Vector2D& a, const Vector2D& b) {
        return a.x * b.y - a.y * b.x;
    }

    // Distance between vectors
    float Vector2D::distance(const Vector2D& a, const Vector2D& b) {
        return (b - a).magnitude();
    }

    // Squared distance
    float Vector2D::distanceSquared(const Vector2D& a, const Vector2D& b) {
        return (b - a).magnitudeSquared();
    }

    // Linear interpolation
    Vector2D Vector2D::lerp(const Vector2D& a, const Vector2D& b, float t) {
        // Clamp t to [0, 1]
        t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
        return a + (b - a) * t;
    }

    // Stream operator
    std::ostream& operator<<(std::ostream& os, const Vector2D& vec) {
        os << "Vector2D(" << vec.x << ", " << vec.y << ")";
        return os;
    }

    // Global scalar multiplication
    Vector2D operator*(float scalar, const Vector2D& vec) {
        return vec * scalar;
    }

} // end of namespace gam300