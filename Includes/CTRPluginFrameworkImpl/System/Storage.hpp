#ifndef CTRPLUGINFRAMEWORKIMPL_STORAGE_HPP
#define CTRPLUGINFRAMEWORKIMPL_STORAGE_HPP

#include <cstring>
#include "types.h"

namespace CTRPluginFramework
{
    // Simplistic container class, zero safety
    template <typename T>
    class Storage
    {
    public:

        /*Storage() :
            _itemCount(0)
        {
            _pool = new T[10];
            if (_pool != nullptr)
                _capacity = 10;
            else
                _capacity = 0;
        }*/

        Storage(u32 capacity) :
            _itemCount(0)
        {
            _pool = new T[capacity];
            if (_pool != nullptr)
                _capacity = capacity;
            else
                _capacity = 0;
        }

        ~Storage()
        {
            if (_pool != nullptr)
                delete[] _pool;
        }

        T     *begin(void)
        {
            if (_pool == nullptr)
                return (nullptr);

            return (&_pool[0]);
        }

        T     *end(void)
        {
            if (_pool == nullptr)
                return (nullptr);

            return (&_pool[_itemCount]);
        }

        void  *data(void)
        {
            return ((void *)_pool);
        }

        // Force item count
        void    resize(u32 size)
        {
            if (size > _capacity)
            {
                T   *newPool = new T[size];

                if (newPool != nullptr)
                {
                    std::memcpy(newPool, _pool, sizeof(T) * _itemCount);
                    delete[] _pool;
                    _pool = newPool;
                    _itemCount = size;
                    _capacity = size;
                }
            }
            else
            {
                _itemCount = size;
            }
        }

        u32     size(void)
        {
            return (_itemCount);
        }

        u32     capacity(void)
        {
            return (_capacity);
        }

        bool    empty(void)
        {
            return (_itemCount == 0);
        }

        T       &operator [](u32 index)
        {
            return (_pool[index]);
        }

        void    push_back(T &&item)
        {
            if (_itemCount == _capacity)
                return;
            _pool[_itemCount] = item;
            _itemCount++;
        }

    private:
        T       *_pool;
        u32     _capacity;
        u32     _itemCount;
    };
}

#endif