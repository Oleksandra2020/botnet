#include "victim_manipulation.h"


void victims::add_victim(const char* iph_destip, const char* tcph_destport)
{
    src_dst ports_ips{};
    ports_ips.source_ip = source_ip;
    ports_ips.source_port = source_port;
    ports_ips.dest_ip = iph_destip;
    ports_ips.dest_port = tcph_destport;
    pool.enqueue(ports_ips);
}

void victims::remove_victim(const char* dest_ip, const char* dest_port)
{
    src_dst to_remove{};
    to_remove.source_ip = source_ip;
    to_remove.source_port = source_port;
    to_remove.dest_ip = dest_ip;
    to_remove.dest_port = dest_port;
    pool.remove_victim(to_remove);
}

void victims::clear_thread_pool(int threads)
{
    for (int thread = 0; thread < threads; thread++)
    {
        pool.insert_pill(thread);
    }
}

void victims::clear_thread(int thread)
{
    pool.insert_pill(thread);
}

