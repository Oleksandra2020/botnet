#ifndef EXAMPLE_SYN_VICTIM_MANIPULATION_H
#define EXAMPLE_SYN_VICTIM_MANIPULATION_H

#include "thread_pool.h"
#include "thread"
#include <algorithm>


class victims
{
public:
    int source_ip;
    int source_port;

    victims(int threads, const char* iph_sourceip, const char* tcph_srcport) : pool(threads)
    {
	source_ip = inet_addr(iph_sourceip);
	source_port = stoi(std::string(tcph_srcport));
    }

    void add_tcp_victim(const char* iph_destip, const char* tcph_destport);

    void add_http_victim(const char* host_name);

    void remove_tcp_victim(const char* dest_ip, const char* dest_port);

    void remove_http_victim(const char* host_name);

    void clear_thread_pool(int threads);


private:
	thread_pool pool;

};

#endif //EXAMPLE_SYN_VICTIM_MANIPULATION_H