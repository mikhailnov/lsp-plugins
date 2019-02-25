/*
 * Mutex.h
 *
 *  Created on: 25 февр. 2019 г.
 *      Author: sadko
 */

#ifndef INCLUDE_CORE_IPC_MUTEX_H_
#define INCLUDE_CORE_IPC_MUTEX_H_

#include <core/types.h>
#include <dsp/atomic.h>

#if defined(PLATFORM_WINDOWS)
    #include <synchapi.h>
#elif defined(PLATFORM_LINUX)
    #include <linux/futex.h>
    #include <sys/syscall.h>
    #include <unistd.h>
    #include <pthread.h>
#else
    #include <pthread.h>
#endif

namespace lsp
{
    namespace ipc
    {
#if defined(PLATFORM_WINDOWS)
        /**
         * Recursive mutex for Windows platform
         */
        class Mutex
        {
            private:
                mutable HANDLE                  hMutex;     // Mutex object
                mutable DWORD                   nThreadId;  // Owner's thread identifier
                mutable atomic_t                nLocks;     // Number of locks by current thread

            private:
                Mutex & operator = (const Mutex & m);       // Deny copying

            public:
                explicit Mutex();
                ~Mutex();

                /** Wait until mutex is unlocked and lock it
                 *
                 */
                bool lock() const;

                /** Try to lock mutex and return status of operation
                 *
                 * @return non-zero value if mutex was locked
                 */
                bool try_lock() const;

                /** Unlock mutex
                 *
                 */
                bool unlock() const;
        };
#elif defined(PLATFORM_LINUX)
        /** Fast recursive mutex implementation for Linux
         *
         */
        class Mutex
        {
            private:
                mutable volatile atomic_t       nLock;      // 1 = locked, 0 = locked
                mutable volatile atomic_t       nWaiters;   // Number of waiters
                mutable pthread_t               nThreadId;  // Locked thread identifier
                mutable atomic_t                nLocks;     // Number of locks by current thread

            private:
                Mutex & operator = (const Mutex & m);       // Deny copying

            public:
                explicit Mutex()
                {
                    nLock       = 1;
                    nWaiters    = 0;
                    nThreadId   = -1;
                    nLocks      = 0;
                }

                /** Wait until mutex is unlocked and lock it
                 *
                 */
                void lock() const;

                /** Try to lock mutex and return status of operation
                 *
                 * @return non-zero value if mutex was locked
                 */
                inline bool try_lock() const
                {
                    // Perform test-and-set lock
                    if (!nLock)
                        return false;
                    if (!atomic_swap(&nLock, 0))
                        return false;

                    // Update recursive counter
                    if (!(nLocks++))
                        nThreadId       = pthread_self(); // Save thread identifier
                    return true;
                }

                /** Unlock mutex
                 *
                 */
                inline bool unlock() const
                {
                    if (nThreadId != pthread_self())
                        return false;
                    if (!(--nLocks))
                    {
                        nThreadId       = -1;
                        nLock           = 1;
                        if (nWaiters > 0)
                            syscall(SYS_futex, &nLock, FUTEX_WAKE, 1, NULL, 0, 0);
                    }
                    return true;
                }
        };
#else
        /**
         * Recursive mutex implementation using pthread
         */
        class Mutex
        {
            private:
                mutable pthread_mutex_t     sMutex;

            private:
                Mutex & operator = (const Mutex & m);       // Deny copying

            public:
                explicit Mutex();
                ~Mutex();

                /** Wait until mutex is unlocked and lock it
                 *
                 */
                inline bool lock() const
                {
                    while (true)
                    {
                        switch (pthread_mutex_lock(&sMutex))
                        {
                            case 0: return true;
                            case EBUSY: pthread_yield(); break;
                            default: return false;
                        }
                    }
                }

                /** Try to lock mutex and return status of operation
                 *
                 * @return non-zero value if mutex was locked
                 */
                inline bool try_lock() const
                {
                    return pthread_mutex_trylock(&sMutex) == 0;
                }

                /** Unlock mutex
                 *
                 */
                inline bool unlock() const
                {
                    pthread_mutex_unlock(&sMutex) == 0;
                }
        };
#endif
    
    } /* namespace ipc */
} /* namespace lsp */

#endif /* INCLUDE_CORE_IPC_MUTEX_H_ */
