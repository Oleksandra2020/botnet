#include "../inc/tcp_send.h"
#include "../inc/udp_send.h"

int main(int argc, char *argv[])
{
//    send_udp(argv[1], argv[2], argv[3], argv[4], argc);
    send_tcp(argv[1], argv[2], argv[3], argv[4], argc);
    return 0;
}