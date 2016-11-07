///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <http://sourcey.com>
//
// SPDX-License-Identifier:	LGPL-2.1+
//
/// @addtogroup base
/// @{


#ifndef SCY_Thread_H
#define SCY_Thread_H


#include "scy/uv/uvpp.h"
#include "scy/mutex.h"
#include "scy/platform.h"
#include "scy/async.h"


namespace scy {


/// This class implements a platform-independent
/// wrapper around an operating system thread.
class Thread: public async::Runner
{
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread();
    Thread(async::Runnable& target);
    Thread(std::function<void()> target);
    Thread(std::function<void(void*)> target, void* arg);
    virtual ~Thread();

    /// Waits until the thread exits.
    void join();

    /// Waits until the thread exits.
    /// The thread should be cancelled beore calling this method.
    /// This method must be called from outside the current thread
    /// context or deadlock will ensue.
    bool waitForExit(int timeout = 5000);

    /// Returns the native thread handle.
    uv_thread_t id() const;

    /// Returns the native thread ID of the current thread.
    static uv_thread_t currentID();

    static const uv_thread_t mainID;

protected:
    Thread(const Thread&);
    Thread& operator = (const Thread&);

    virtual bool async() const;
    virtual void startAsync();

    uv_thread_t _handle;
};


//
// Runner Startable
//

/// Depreciated: This class is an invisible wrapper around a TStartable instance,
/// which provides asynchronous access to the TStartable start() and
/// stop() methods. TStartable is an instance of async::Startable.
template <class TStartable>
class AsyncStartable: public TStartable
{
public:
    AsyncStartable() {};
    virtual ~AsyncStartable() {};

    static void runAsync(void* arg) {
        try {
            // Call the blocking start() function once only
            static_cast<TStartable*>(arg)->start();
        }
        catch (std::exception& exc) {
            // errorL("AsyncStartable") << exc.what() << std::endl;
#ifdef _DEBUG
            throw exc;
#endif
        }
    }

    virtual bool start()
    {
        _thread.start(*this);
        return true;
    }

    virtual void stop()
    {
        TStartable::stop();
        _thread.join();
    }

protected:
    Thread _thread;
};


} // namespace scy


#endif

/// @\}
