#ifndef CTRPLUGINFRAMEWORK_RECT_HPP
#define CTRPLUGINFRAMEWORK_RECT_HPP

#include <Vector.hpp>
#include <algorithm>

namespace CTRPluginFramework
{
    template <typename T>
    class Rect
    {
    public:
        Rect();
        Rect(const Vector<T>& leftTopCorner, const Vector<T>& size);
        Rect(const Vector<T>& leftTopCorner, T width, T height);
        Rect(T left, T top, const Vector<T>& size);
        Rect(T left, T top, T width, T height);
        template <typename U>
        explicit Rect(const Rect<U>& rect);

        bool Contains(T x, T y);
        bool Contains(const Vector<T>& point);
        bool Intersects(const Rect<T>& rect);
        bool Intersects(const Rect<T>& rect, Rect<T>& intersect);

        Vector<T> _leftTopCorner;
        Vector<T> _size;
    };

    template <typename T>
    Rect<T>::Rect() : _leftTopCorner(0, 0), _size(0, 0)
    {

    }

    template <typename T>
    Rect<T>::Rect(const Vector<T>& leftTopCorner, const Vector<T>& size)
    {
        _leftTopCorner = leftTopCorner;
        _size = size;
    }

    template <typename T>
    Rect<T>::Rect(const Vector<T>& leftTopCorner, T width, T height)
    {
        _leftTopCorner = leftTopCorner;
        _size.x = width;
        _size.y = height;
    }

    template <typename T>
    Rect<T>::Rect(T left, T top, const Vector<T>& size)
    {
        _leftTopCorner.x = left;
        _leftTopCorner.y = top;
        _size = size;
    }

    template <typename T>
    Rect<T>::Rect(T left, T top, T width, T height)
    {
        _leftTopCorner.x = left;
        _leftTopCorner.y = top;
        _size.x = width;
        _size.y = height;
    }

    template <typename T>
    template <typename U>
    Rect<T>::Rect(const Rect<U>& rect)
    {
        _leftTopCorner = reinterpret_cast<T>(rect._leftTopCorner);
        _size = reinterpret_cast<T>(rect._size);
    }

    template <typename T>
    bool Rect<T>::Contains(T x, T y)
    {
        T minX = std::min(_leftTopCorner.x, _leftTopCorner.x + _size.x);
        T maxX = std::max(_leftTopCorner.x, _leftTopCorner.x + _size.x);
        T minY = std::min(_leftTopCorner.y, _leftTopCorner.y + _size.y);
        T maxY = std::max(_leftTopCorner.y, _leftTopCorner.y + _size.y);

        return (x >= minX && x < maxX
            && y >= minY && y < maxY);
    }

    template <typename T>
    bool Rect<T>::Contains(const Vector<T>& point)
    {
        return (Contains(point.x, point.y));
    }

    template <typename T>
    bool Rect<T>::Intersects(const Rect<T>& rect)
    {
        Rect<T> intersect;
        return (Intersects(rect, intersect));
    }

    template <typename T>
    bool Rect<T>::Intersects(const Rect<T> &rect, Rect<T> &intersect)
    {
        T thisMinX = std::min(_leftTopCorner.x, _leftTopCorner.x + _size.x);
        T thisMaxX = std::max(_leftTopCorner.x, _leftTopCorner.x + _size.x);
        T thisMinY = std::min(_leftTopCorner.y, _leftTopCorner.y + _size.y);
        T thisMaxY = std::max(_leftTopCorner.y, _leftTopCorner.y + _size.y);
        T rectMinX = std::min(rect._leftTopCorner.x, rect._leftTopCorner.x + rect._size.x);
        T rectMaxX = std::max(rect._leftTopCorner.x, rect._leftTopCorner.x + rect._size.x);
        T rectMinY = std::min(rect._leftTopCorner.y, rect._leftTopCorner.y + rect._size.y);
        T rectMaxY = std::max(rect._leftTopCorner.y, rect._leftTopCorner.y + rect._size.y);

        T intersectLeftX = std::max(thisMinX, rectMinX);
        T intersectLeftY = std::max(thisMinY, rectMinY);
        T intersectRightX = std::min(thisMaxX, rectMaxX);
        T intersectRightY = std::min(thisMaxY, rectMaxY);

        if (intersectLeftX < intersectRightX && intersectLeftY < intersectRightY)
        {
            intersect = Rect<T>(intersectLeftX, intersectLeftY, intersectRightX - intersectLeftX,
                                intersectRightY - intersectLeftY);
            return (true);
        }
        intersect = Rect<T>(0, 0, 0, 0);
        return (false);
    }

    template <typename T>
    bool operator ==(Rect<T> &left, Rect<T> &right)
    {
        return (left._leftTopCorner == right._leftTopCorner
                && left._size == right._size);
    }

    template <typename T>
    bool operator !=(Rect<T> &left, Rect<T> &right)
    {
        return (left._leftTopCorner != right._leftTopCorner
            && left._size != right._size);
    }

    typedef Rect<unsigned int> UIntRect;
    typedef Rect<int> IntRect;
    typedef Rect<float> FloatRect;
}

#endif