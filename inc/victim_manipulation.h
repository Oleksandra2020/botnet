#ifndef EXAMPLE_SYN_VICTIM_MANIPULATION_H
#define EXAMPLE_SYN_VICTIM_MANIPULATION_H

#include "tcp_send.h"
#include "thread"
#include "thread_pool.h"

class victims {
    public:
	thread_pool pool;
	TCPSend send;

	victims(int threads) : pool(threads), send() {}

	void add_victim(const char* iph_sourceip, const char* tcph_srcport, const char* iph_destip, const char* tcph_destport);

	void remove_victim(const char* dest_ip, const char* dest_port);

	void clear_thread_pool(int threads);

	void clear_thread(int thread);

    private:
};

#endif	// EXAMPLE_SYN_VICTIM_MANIPULATION_H