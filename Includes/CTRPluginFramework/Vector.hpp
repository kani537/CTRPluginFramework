﻿#pragma once

#include "types.h"

namespace CTRPluginFramework
{
    template <typename T>
    class Vector
    {
    public:
        Vector();
        Vector(T x, T y);
        template <typename U>
        explicit Vector(const Vector<U> &vector);

        T x;
        T y;
    };

    template <typename T>
    Vector<T>::Vector() : x(0), y(0)
    {

    }

    template <typename T>
    Vector<T>::Vector(T x, T y) : x(x), y(y)
    {

    }

    template <typename T>
    template <typename U>
    Vector<T>::Vector(const Vector<U> &vector) : x(static_cast<T>(vector.x)), y(static_cast<T>(vector.y))
    {

    }

    template <typename T>
    Vector<T> operator - (const Vector<T> &vector)
    {
        return (Vector<T>(-vector.x, -vector.y));
    }

    template <typename T>
    Vector<T> operator - (const Vector<T> &left, const Vector<T> &right)
    {
        return (Vector<T>(left.x - right.x, left.y - right.y));
    }

    template <typename T>
    Vector<T> &operator -= (const Vector<T> &left, const Vector<T> &right)
    {
        left.x -= right.x;
        left.y -= right.y;
        return (left);
    }

    template <typename T>
    Vector<T> operator + (const Vector<T> &left, const Vector<T> &right)
    {
        return (Vector<T>(left.x + right.x, left.y + right.y));
    }

    template <typename T>
    Vector<T> &operator += (const Vector<T> &left, const Vector<T> &right)
    {
        left.x += right.x;
        left.y += right.y;
        return (left);
    }

    template <typename T>
    Vector<T> operator * (const Vector<T> &left, T right)
    {
        return (Vector<T>(left.x * right, left.y * right));
    }

    template <typename T>
    Vector<T> operator * (T left, const Vector<T> &right)
    {
        return (Vector<T>(right.x * left, right.y * left));
    }

    template <typename T>
    Vector<T> &operator *= (Vector<T> &left, T right)
    {
        left.x *= right;
        left.y *= right;
        return (left);
    }

    template <typename T>
    Vector<T> operator / (Vector<T> &left, T right)
    {
        return (Vector<T>(left.x / right, left.y / right));
    }

    template <typename T>
    Vector<T> &operator /= (Vector<T> &left, T right)
    {
        left.x /= right;
        left.y /= right;
        return (left);
    }

    template <typename T>
    bool operator <= (Vector<T> &left, Vector<T> &right)
    {
        return (left.x <= right.x
            && left.y <= right.y);
    } 

    template <typename T>
    bool operator >= (Vector<T> &left, Vector<T> &right)
    {
        return (left.x >= right.x
            && left.y >= right.y);
    } 

    typedef Vector<unsigned int> UIntVector;
    typedef Vector<int> IntVector;
    typedef Vector<float> FloatVector;
}
