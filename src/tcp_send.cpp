//---cat rawtcp.c---
// Run as root or SUID 0, just datagram no data/payload
#include "../inc/tcp_send.h"


// Simple checksum function, may use others such as Cyclic Redundancy Check, CRC
unsigned short csum(unsigned short *buf, int len)
{
    unsigned long sum;
    for(sum=0; len>0; len--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int send_tcp(const char* iph_sourceip, const char* tcph_srcport, const char* iph_destip, const char* tcph_destport, int argc)
{
    int sd;
// No data, just datagram
    char buffer[PCKT_LEN];
// The size of the headers
    struct ipheader *ip = (struct ipheader *) buffer;
    struct tcpheader *tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));
    struct sockaddr_in sin, din;
    int one = 1;
    const int *val = &one;

    memset(buffer, 0, PCKT_LEN);

    if(argc != 5)
    {
        printf("- Invalid parameters!!!\n");
        printf("- Usage: <source hostname/IP> <source port> <target hostname/IP> <target port>\n");
        exit(-1);
    }

    sd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if(sd < 0)
    {
        perror("socket() error");
        exit(-1);
    }
    else
        printf("socket()-SOCK_RAW and tcp protocol is OK.\n");

// The source is redundant, may be used later if needed
// Address family
    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;
// Source port, can be any, modify as needed
    sin.sin_port = htons(atoi(tcph_srcport));
    din.sin_port = htons(atoi(tcph_destport));
// Source IP, can be any, modify as needed
    sin.sin_addr.s_addr = inet_addr(iph_sourceip);
    din.sin_addr.s_addr = inet_addr(iph_destip);
// IP structure
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 16;
    ip->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
    ip->iph_ident = htons(54321);
    ip->iph_offset = 0;
    ip->iph_ttl = 64;
    ip->iph_protocol = 6; // TCP
    ip->iph_chksum = 0; // Done by kernel

// Source IP, modify as needed, spoofed, we accept through command line argument
    ip->iph_sourceip = inet_addr(iph_sourceip);
// Destination IP, modify as needed, but here we accept through command line argument
    ip->iph_destip = inet_addr(iph_destip);

// The TCP structure. The source port, spoofed, we accept through the command line
    tcp->tcph_srcport = htons(atoi(tcph_srcport));
// The destination port, we accept through command line
    tcp->tcph_destport = htons(atoi(tcph_destport));
    tcp->tcph_seqnum = htonl(1);
    tcp->tcph_acknum = 0;
    tcp->tcph_offset = 5;
    tcp->tcph_syn = 1;
    tcp->tcph_ack = 0;
    tcp->tcph_win = htons(32767);
    tcp->tcph_chksum = 0; // Done by kernel
    tcp->tcph_urgptr = 0;
// IP checksum calculation
    ip->iph_chksum = csum((unsigned short *) buffer, (sizeof(struct ipheader) + sizeof(struct tcpheader)));

// Inform the kernel do not fill up the headers' structure, we fabricated our own

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR , val, sizeof(one)))
    {
        perror("setsockopt() error");
        exit(-1);
    }
    else
        printf("setsockopt() is OK\n");

    printf("Using:::::Source IP: %s port: %u, Target IP: %s port: %u.\n", iph_sourceip, atoi(tcph_srcport), iph_destip, atoi(tcph_destport));

// sendto() loop, send every 2 second for 50 counts
    unsigned int count;
    for(count = 0; count < 20; count++)
    {
        if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
// Verify
        {
            perror("sendto() error");
            exit(-1);
        }
        else
            printf("Count #%u - sendto() is OK\n", count);
        sleep(2);
    }
    close(sd);
    return 0;
}