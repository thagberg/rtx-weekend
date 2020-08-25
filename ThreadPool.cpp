//
// Created by timot on 8/25/2020.
//

#include "ThreadPool.h"

namespace hvk
{
    WorkQueue::WorkQueue()
        : mMutex()
        , mQueue()
    {

    }

    WorkQueue::~WorkQueue()
    {

    }

    void WorkQueue::push(Proc p)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mQueue.push(p);
    }

    WorkQueue::Proc WorkQueue::pop()
    {
        while (true)
        {
            while(mQueue.empty())
            {

            }

            std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);
            if (lock.try_lock())
            {
                if (mQueue.empty())
                {
                    continue;
                }
                auto p = mQueue.front();
                mQueue.pop();
                return p;
            }
            else
            {
                continue;
            }
        }

        return nullptr;
    }

    ThreadPool::ThreadPool(size_t numThreads)
    : mPool()
    , mQueue()
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            mPool.emplace_back(std::thread([&]() {
                while (true)
                {
                    auto job = mQueue.pop();
                    if (job == nullptr)
                    {
                        mQueue.push(nullptr);
                        break;
                    }
                    job();
                }
            }));
        }
    }

    ThreadPool::~ThreadPool()
    {
        mQueue.push(nullptr);
        for (auto & t : mPool)
        {
            t.join();
        }
    }
}
