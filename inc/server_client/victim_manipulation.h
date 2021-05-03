#ifndef EXAMPLE_SYN_VICTIM_MANIPULATION_H
#define EXAMPLE_SYN_VICTIM_MANIPULATION_H

#include "thread_pool.h"
#include "thread"

class victims
{
public:
    thread_pool pool;
    const char* source_ip;
    const char* source_port;

    victims(int threads, const char* iph_sourceip, const char* tcph_srcport) : pool(threads)
    {
        source_ip = iph_sourceip;
        source_port = tcph_srcport;
    }

    void add_victim(const char* iph_destip, const char* tcph_destport);

    void remove_victim(const char* dest_ip, const char* dest_port);

    void clear_thread_pool(int threads);

    void clear_thread(int thread);

private:

};

#endif //EXAMPLE_SYN_VICTIM_MANIPULATION_H
