//
//  ThreadRWLock.cpp
//  videosystem
//
//  Created by SDK Team on 2023/3/2.
//

#include "ThreadRWLock.hpp"
#include "CppVersion.h"

#if SUPPORT_CPP17
#include <shared_mutex>
#else
#include <pthread.h>
#endif

namespace task
{

#if SUPPORT_CPP17

ThreadRWLock::ThreadRWLock()
{
    mRWLock = static_cast<void*>(new std::shared_mutex);
}

ThreadRWLock::~ThreadRWLock()
{
    if (mRWLock)
    {
        delete (std::shared_mutex*)mRWLock;
        mRWLock = nullptr;
    }
}

void ThreadRWLock::readLock()
{
    static_cast<std::shared_mutex*>(mRWLock)->lock_shared();
}

void ThreadRWLock::writeLock()
{
    static_cast<std::shared_mutex*>(mRWLock)->lock();
}

void ThreadRWLock::unReadLock()
{
    static_cast<std::shared_mutex*>(mRWLock)->unlock_shared();
}

void ThreadRWLock::unWriteLock()
{
    static_cast<std::shared_mutex*>(mRWLock)->unlock();
}

#else

ThreadRWLock::ThreadRWLock()
{
    mRWLock = new pthread_rwlock_t;
    pthread_rwlock_init(static_cast<pthread_rwlock_t*>(mRWLock), nullptr);
}

ThreadRWLock::~ThreadRWLock()
{
    if (mRWLock)
    {
        pthread_rwlock_destroy(static_cast<pthread_rwlock_t*>(mRWLock));
        delete static_cast<pthread_rwlock_t*>(mRWLock);
        mRWLock = nullptr;
    }
}

void ThreadRWLock::readLock()
{
    pthread_rwlock_rdlock(static_cast<pthread_rwlock_t*>(mRWLock));
}

void ThreadRWLock::writeLock()
{
    pthread_rwlock_wrlock(static_cast<pthread_rwlock_t*>(mRWLock));
}

void ThreadRWLock::unReadLock()
{
    pthread_rwlock_unlock(static_cast<pthread_rwlock_t*>(mRWLock));
}

void ThreadRWLock::unWriteLock()
{
    pthread_rwlock_unlock(static_cast<pthread_rwlock_t*>(mRWLock));
}

#endif

}  // namespace task
