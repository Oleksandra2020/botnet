#ifndef EXAMPLE_SYN_THREAD_POOL_H
#define EXAMPLE_SYN_THREAD_POOL_H

#include <condition_variable>
#include <future>
#include <queue>
#include <set>
#include <unordered_map>

#include "tbb/concurrent_unordered_set.h"

class thread_pool {
    public:
	explicit thread_pool(int threads) { start(threads); }

	~thread_pool() { stop(); }

	void enqueue(std::function<void()> task) {
		{
			std::unique_lock<std::mutex> lock{event_mutex};

			size_t target = 0;
			size_t min_size = tasks_per_thread[target].size();
			for (int i = 0; i < tasks_per_thread.size(); i++) {
				size_t cur_size = tasks_per_thread[i].size();
				if (cur_size < min_size) {
					min_size = cur_size;
					target = i;
				}
			}
			tasks_per_thread[target].push_back(std::move(task));
		}
		event_char_m.notify_one();
	}

	void insert_pill(int thread_num) { tasks_per_thread[thread_num].push_front(nullptr); }

    private:
	std::vector<std::thread> threads_m;
	std::condition_variable event_char_m;
	std::mutex event_mutex;
	bool stop_m = false;
	std::unordered_map<size_t, std::deque<std::function<void()>>> tasks_per_thread;

	void start(int threads) {
		for (auto i = 0; i < threads; i++) {
			threads_m.emplace_back([=] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock{event_mutex};
						event_char_m.wait(lock, [=] { return stop_m || !tasks_per_thread[i].empty(); });
						if (stop_m && tasks_per_thread[i].empty()) break;

						task = std::move(tasks_per_thread[i].front());
						tasks_per_thread[i].pop_front();
						if (task == nullptr) {
							break;
						}
						tasks_per_thread[i].push_back(task);
					}
					task();
				}
			});
		}
	}

	void stop() noexcept {
		{
			std::unique_lock<std::mutex> lock{event_mutex};
			stop_m = true;
		}
		event_char_m.notify_all();
		for (auto& thread : threads_m) {
			thread.join();
		}
		threads_m.clear();
	}
};

#endif	// EXAMPLE_SYN_THREAD_POOL_H