#ifndef RTX_WEEKEND_THREADPOOL_H
#define RTX_WEEKEND_THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>

namespace hvk
{
    class WorkQueue
    {
        using Proc = std::function<void(void)>;
        using Queue = std::queue<Proc>;

    public:
        WorkQueue();
        ~WorkQueue();

        void push(Proc p);
        Proc pop();


    private:
        std::mutex mMutex;
        Queue mQueue;
    };

    class ThreadPool
    {
        using Pool = std::vector<std::thread>;
        using Proc = std::function<void(void)>;

    public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        template<typename F, typename... Args>
        void QueueWork(F&& f, Args&&... args)
        {
            mQueue.push([=]() { f(args...); });
        }

    private:
        Pool mPool;
        WorkQueue mQueue;
    };
}

#endif //RTX_WEEKEND_THREADPOOL_H

