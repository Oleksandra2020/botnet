#ifndef EXAMPLE_SYN_THREAD_POOL_H
#define EXAMPLE_SYN_THREAD_POOL_H

#include <condition_variable>
#include <queue>
#include <future>


class ThreadPool
{
public:

    explicit ThreadPool(int threads)
    {
        start(threads);
    }

    ~ThreadPool()
    {
        stop();
    }

    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock{event_mutex};
            tasks_m.emplace(std::move(task));
        }
        event_char_m.notify_one();
    }


private:
    std::vector<std::thread> threads_m;
    std::condition_variable event_char_m;
    std::mutex event_mutex;
    bool stop_m = false;
    std::queue<std::function<void()>> tasks_m;

    void start(int threads)
    {
        for (auto i = 0; i < threads; i++)
        {
            threads_m.emplace_back([=] {
                while(true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock{event_mutex};
                        event_char_m.wait(lock, [=] {return stop_m || !tasks_m.empty();});
                        if (stop_m && tasks_m.empty())
                            break;

                        task = std::move(tasks_m.front());
                        tasks_m.pop();
                    }
                    task();
                }
            });
        }
    }

    void stop() noexcept
    {
        {
            std::unique_lock<std::mutex> lock{event_mutex};
            stop_m = true;
        }
        event_char_m.notify_all();
        for (auto& thread: threads_m)
        {
            thread.join();
        }
        threads_m.clear();
    }
};

#endif //EXAMPLE_SYN_THREAD_POOL_H
