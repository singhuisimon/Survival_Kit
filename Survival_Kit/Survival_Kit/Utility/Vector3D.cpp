/**
 * @file Vector3D.cpp
 * @brief Implementation of the Vector3D class for the game engine.
 * @details Contains implementations for all member functions declared in Vector3D.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Vector3D.h"

namespace gam300 {

    // Initialize static constants
    const Vector3D Vector3D::ZERO(0.0f, 0.0f, 0.0f);
    const Vector3D Vector3D::ONE(1.0f, 1.0f, 1.0f);
    const Vector3D Vector3D::UNIT_X(1.0f, 0.0f, 0.0f);
    const Vector3D Vector3D::UNIT_Y(0.0f, 1.0f, 0.0f);
    const Vector3D Vector3D::UNIT_Z(0.0f, 0.0f, 1.0f);
    const Vector3D Vector3D::UP(0.0f, 1.0f, 0.0f);      // Same as UNIT_Y
    const Vector3D Vector3D::DOWN(0.0f, -1.0f, 0.0f);
    const Vector3D Vector3D::RIGHT(1.0f, 0.0f, 0.0f);   // Same as UNIT_X
    const Vector3D Vector3D::LEFT(-1.0f, 0.0f, 0.0f);
    const Vector3D Vector3D::FORWARD(0.0f, 0.0f, -1.0f); // OpenGL convention
    const Vector3D Vector3D::BACK(0.0f, 0.0f, 1.0f);

    // Default constructor
    Vector3D::Vector3D() : x(0.0f), y(0.0f), z(0.0f) {}

    // Constructor with components
    Vector3D::Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

    // Copy constructor
    Vector3D::Vector3D(const Vector3D& other) : x(other.x), y(other.y), z(other.z) {}

    // Construct from Vector2D + z component
    Vector3D::Vector3D(const Vector2D& vec2, float z) : x(vec2.x), y(vec2.y), z(z) {}

    // Convert to Vector2D (drop z)
    Vector2D Vector3D::toVector2D() const {
        return Vector2D(x, y);
    }

    // Assignment operator
    Vector3D& Vector3D::operator=(const Vector3D& other) {
        // Check for self-assignment
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }

    // Addition
    Vector3D Vector3D::operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    // Subtraction
    Vector3D Vector3D::operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    // Scalar multiplication
    Vector3D Vector3D::operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    // Scalar division
    Vector3D Vector3D::operator/(float scalar) const {
        // Check for division by zero
        if (scalar != 0.0f) {
            float invScalar = 1.0f / scalar;
            return Vector3D(x * invScalar, y * invScalar, z * invScalar);
        }
        return *this; // Return original vector on division by zero
    }

    // Compound addition
    Vector3D& Vector3D::operator+=(const Vector3D& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    // Compound subtraction
    Vector3D& Vector3D::operator-=(const Vector3D& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    // Compound scalar multiplication
    Vector3D& Vector3D::operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    // Compound scalar division
    Vector3D& Vector3D::operator/=(float scalar) {
        // Check for division by zero
        if (scalar != 0.0f) {
            float invScalar = 1.0f / scalar;
            x *= invScalar;
            y *= invScalar;
            z *= invScalar;
        }
        return *this;
    }

    // Negation
    Vector3D Vector3D::operator-() const {
        return Vector3D(-x, -y, -z);
    }

    // Equality
    bool Vector3D::operator==(const Vector3D& other) const {
        // Use epsilon comparison for floating-point values
        const float EPSILON = 0.000001f;
        return (std::abs(x - other.x) < EPSILON &&
            std::abs(y - other.y) < EPSILON &&
            std::abs(z - other.z) < EPSILON);
    }

    // Inequality
    bool Vector3D::operator!=(const Vector3D& other) const {
        return !(*this == other);
    }

    // Magnitude (length)
    float Vector3D::magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Squared magnitude
    float Vector3D::magnitudeSquared() const {
        return x * x + y * y + z * z;
    }

    // Normalize (return a unit vector)
    Vector3D Vector3D::normalize() const {
        float mag = magnitude();
        if (mag > 0.0f) {
            float invMag = 1.0f / mag;
            return Vector3D(x * invMag, y * invMag, z * invMag);
        }
        return *this; // Return original vector if magnitude is zero
    }

    // Normalize in place
    void Vector3D::normalizeInPlace() {
        float mag = magnitude();
        if (mag > 0.0f) {
            float invMag = 1.0f / mag;
            x *= invMag;
            y *= invMag;
            z *= invMag;
        }
    }

    // Dot product
    float Vector3D::dot(const Vector3D& a, const Vector3D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // Cross product
    Vector3D Vector3D::cross(const Vector3D& a, const Vector3D& b) {
        return Vector3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    // Distance between vectors
    float Vector3D::distance(const Vector3D& a, const Vector3D& b) {
        return (b - a).magnitude();
    }

    // Squared distance
    float Vector3D::distanceSquared(const Vector3D& a, const Vector3D& b) {
        return (b - a).magnitudeSquared();
    }

    // Linear interpolation
    Vector3D Vector3D::lerp(const Vector3D& a, const Vector3D& b, float t) {
        // Clamp t to [0, 1]
        t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
        return a + (b - a) * t;
    }

    // Project v onto 'onto'
    Vector3D Vector3D::project(const Vector3D& v, const Vector3D& onto) {
        float magnitudeSq = onto.magnitudeSquared();
        if (magnitudeSq < 0.000001f) {
            return Vector3D::ZERO; // Avoid division by zero
        }

        float dotProduct = dot(v, onto);
        float scale = dotProduct / magnitudeSq;
        return onto * scale;
    }

    // Reflect v about normal
    Vector3D Vector3D::reflect(const Vector3D& v, const Vector3D& normal) {
        // Make sure normal is normalized
        Vector3D normalizedNormal = normal.normalize();

        // r = v - 2(v·n)n
        return v - normalizedNormal * (2.0f * dot(v, normalizedNormal));
    }

    // Stream operator
    std::ostream& operator<<(std::ostream& os, const Vector3D& vec) {
        os << "Vector3D(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }

    // Global scalar multiplication
    Vector3D operator*(float scalar, const Vector3D& vec) {
        return vec * scalar;
    }

} // end of namespace gam300