#include "../inc/victim_manipulation.h"


void victims::add_victim(const char* iph_sourceip, const char* tcph_srcport, const char* iph_destip, const char* tcph_destport)
{
    pool.enqueue([=]
                 {
                     send.send_tcp(iph_sourceip, tcph_srcport, iph_destip, tcph_destport);
                 });
}

void victims::remove_victim(size_t victim_id)
{
}

