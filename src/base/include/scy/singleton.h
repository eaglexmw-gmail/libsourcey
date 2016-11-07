///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <http://sourcey.com>
//
// SPDX-License-Identifier:	LGPL-2.1+
//
/// @addtogroup base
/// @{


#ifndef SCY_Singleton_H
#define SCY_Singleton_H


#include <cstdint>


namespace scy {


/// This is a helper template class for managing
/// singleton objects allocated on the heap.
template <class S>
class Singleton
{
public:
    /// Creates the Singleton wrapper.
    Singleton() : _ptr(0)
    {
    }

    /// Destroys the Singleton wrapper and the
    /// managed singleton instance it holds.
    ~Singleton()
    {
        if (_ptr)
            delete _ptr;
    }

    /// Returns a pointer to the singleton object
    /// hold by the Singleton. The first call
    /// to get on a nullptr singleton will instantiate
    /// the singleton.
    S* get()
    {
        Mutex::ScopedLock lock(_m);
        if (!_ptr)
            _ptr = new S;
        return _ptr;
    }

    /// Swaps the old pointer with the new one and
    /// returns the old instance.
    S* swap(S* newPtr)
    {
        Mutex::ScopedLock lock(_m);
        S* oldPtr = _ptr;
        _ptr = newPtr;
        return oldPtr;
    }

    /// Destroys the managed singleton instance.
    void destroy()
    {
        Mutex::ScopedLock lock(_m);
        if (_ptr)
            delete _ptr;
        _ptr = nullptr;
    }

private:
    S* _ptr;
    Mutex _m;
};


} // namespace scy


#endif // SCY_Singleton_H

/// @\}
