#pragma once
/**
 * @file Vector2D.h
 * @brief Declaration of the Vector2D class for the game engine.
 * @details Provides 2D vector mathematics functionality for positions, velocities, and directions.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __VECTOR2D_H__
#define __VECTOR2D_H__

#include <cmath>
#include <iostream>

namespace gam300 {

    class Vector2D {
    public:
        // Components
        float x;
        float y;

        // Constructors
        Vector2D();                            // Default constructor (0,0)
        Vector2D(float x, float y);            // Constructor with components
        Vector2D(const Vector2D& other);       // Copy constructor

        // Assignment
        Vector2D& operator=(const Vector2D& other);

        // Basic arithmetic operations
        Vector2D operator+(const Vector2D& other) const;
        Vector2D operator-(const Vector2D& other) const;
        Vector2D operator*(float scalar) const;
        Vector2D operator/(float scalar) const;
        Vector2D& operator+=(const Vector2D& other);
        Vector2D& operator-=(const Vector2D& other);
        Vector2D& operator*=(float scalar);
        Vector2D& operator/=(float scalar);

        // Negation
        Vector2D operator-() const;

        // Comparison
        bool operator==(const Vector2D& other) const;
        bool operator!=(const Vector2D& other) const;

        // Vector operations
        float magnitude() const;               // Length of the vector
        float magnitudeSquared() const;        // Squared length (faster when only comparing)
        Vector2D normalize() const;            // Returns a normalized (unit) vector
        void normalizeInPlace();               // Normalizes this vector in place

        // Static vector operations
        static float dot(const Vector2D& a, const Vector2D& b);       // Dot product
        static float cross(const Vector2D& a, const Vector2D& b);     // 2D cross product (returns scalar)
        static float distance(const Vector2D& a, const Vector2D& b);  // Distance between two vectors
        static float distanceSquared(const Vector2D& a, const Vector2D& b); // Squared distance
        static Vector2D lerp(const Vector2D& a, const Vector2D& b, float t); // Linear interpolation

        // Common vectors
        static const Vector2D ZERO;
        static const Vector2D ONE;
        static const Vector2D UNIT_X;
        static const Vector2D UNIT_Y;
    };

    // Stream operators for easy printing
    std::ostream& operator<<(std::ostream& os, const Vector2D& vec);

    // Global scalar multiplication
    Vector2D operator*(float scalar, const Vector2D& vec);

} // end of namespace gam300

#endif // __VECTOR2D_H__