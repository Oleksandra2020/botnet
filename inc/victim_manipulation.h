#ifndef EXAMPLE_SYN_VICTIM_MANIPULATION_H
#define EXAMPLE_SYN_VICTIM_MANIPULATION_H

#include "tcp_send.h"
#include "thread_pool.h"
#include "thread"


class victims
{
public:
    ThreadPool pool;
    TCPSend send;

    victims() : pool(std::thread::hardware_concurrency()), send() {}

    void add_victim(const char* iph_sourceip, const char* tcph_srcport, const char* iph_destip, const char* tcph_destport);

    void remove_victim(size_t victim_id);

private:

};

#endif //EXAMPLE_SYN_VICTIM_MANIPULATION_H
