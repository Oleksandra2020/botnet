
#ifndef EXAMPLE_SYN_UDP_SEND_H
#define EXAMPLE_SYN_UDP_SEND_H

#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <iostream>
#include <arpa/inet.h>

// The packet length
#define PCKT_LEN 8192
#include "../inc/tcp_send.h"


//struct ipheader {
//    unsigned char      iph_ihl:5, iph_ver:4;
//    unsigned char      iph_tos;
//    unsigned short int iph_len;
//    unsigned short int iph_ident;
//    unsigned char      iph_flag;
//    unsigned short int iph_offset;
//    unsigned char      iph_ttl;
//    unsigned char      iph_protocol;
//    unsigned short int iph_chksum;
//    unsigned int       iph_sourceip;
//    unsigned int       iph_destip;
//};

// UDP header's structure
struct udpheader {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
};

int send_udp(const char* iph_sourceip, const char* udph_srcport, const char* iph_destip, const char* udph_destport, int argc);

unsigned short csum(unsigned short *buf, int nwords);

#endif //EXAMPLE_SYN_UDP_SEND_H
