#include "../inc/tcp_send.h"
#include "../inc/thread_pool.h"

int main(int argc, char *argv[])
{
    conf_t conf;
    conf.src_dst[std::make_pair("192.168.0.102", "8080")] = std::make_pair("192.168.0.102", "8080");
    conf.src_dst[std::make_pair("192.168.0.102", "1024")] = std::make_pair("192.168.0.102", "8080");

    conf.threads = 2;

    if (conf.threads == 1)
    {
        for (auto& it: conf.src_dst)
        {
            auto src_ip = it.first.first;
            auto dst_ip = it.second.first;
            auto src_port = it.first.second;
            auto dst_port = it.second.second;
            send_tcp(src_ip, src_port, dst_ip, dst_port);
        }
    } else
    {
        ThreadPool pool(conf.threads);

        for (auto& it: conf.src_dst)
        {
            pool.enqueue([src_ip = it.first.first, dst_ip = it.second.first, src_port = it.first.second, dst_port = it.second.second]
                         {
                send_tcp(src_ip, src_port, dst_ip, dst_port);
                         });
        }
    }

    return 0;
}
