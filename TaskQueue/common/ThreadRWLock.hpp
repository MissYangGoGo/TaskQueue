//
//  ThreadRWLock.hpp
//  videosystem
//
//  Created by SDK Team on 2023/3/2.
//

#pragma once
#include <memory>
namespace task
{

class ThreadRWLock
{
public:
    static std::shared_ptr<ThreadRWLock> create()
    {
        return std::make_shared<ThreadRWLock>();
    }

    ThreadRWLock();
    ~ThreadRWLock();

    void readLock();
    void unReadLock();
    void writeLock();
    void unWriteLock();

private:
    void* mRWLock{ nullptr };
};

class ReadLock
{
public:
    explicit ReadLock(std::shared_ptr<ThreadRWLock> mutex)
        : mMutex(mutex)
    {
        mutex->readLock();
    }

    ~ReadLock()
    {
        mMutex->unReadLock();
    }

private:
    std::shared_ptr<ThreadRWLock> mMutex;
};

class WriteLock
{
public:
    explicit WriteLock(std::shared_ptr<ThreadRWLock> mutex)
        : mMutex(mutex)
    {
        mutex->writeLock();
    }
    ~WriteLock()
    {
        mMutex->unWriteLock();
    }

private:
    std::shared_ptr<ThreadRWLock> mMutex;
};

}  // namespace task
