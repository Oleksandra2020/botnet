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
    auto *ip = (struct ipheader *) buffer;
    auto *tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));
    struct sockaddr_in sin{}, din{};
    int one = 1;
    const int *val = &one;

    memset(buffer, 0, PCKT_LEN);

    if(argc != 5)
    {
        std::cout << "- Invalid parameters!!!" << std::endl;
        std::cout << "- Usage: <source hostname/IP> <source port> <target hostname/IP> <target port>" << std::endl;
        exit(-1);
    }

    sd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(sd < 0)
    {
        perror("socket() error");
        exit(-1);
    }
    else
        std::cout << "socket()-SOCK_RAW and tcp protocol is OK." << std::endl;

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
    ip->iph_ihl = HEADER_LEN;
    ip->iph_ver = VERSION;
    ip->iph_tos = TYPE_OF_SERVICE;
    ip->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
    ip->iph_ident = htons(IDENTIFIER);
    ip->iph_offset = 0;
    ip->iph_ttl = TIME_TO_LIVE;
    ip->iph_protocol = IPPROTO_TCP; // TCP
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
    tcp->tcph_offset = TCP_OFFSET;
    tcp->tcph_syn = TCP_SYN;
    tcp->tcph_ack = 0;
    tcp->tcph_win = htons(TCP_WIN);
    tcp->tcph_chksum = 0; // Done by kernel
    tcp->tcph_urgptr = 0;
// IP checksum calculation
    ip->iph_chksum = csum((unsigned short *) buffer, (sizeof(struct ipheader) + sizeof(struct tcpheader)));

// Inform the kernel do not fill up the headers' structure, we fabricated our own

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR , val, sizeof(one)))
    {
        perror("setsockopt() error");
        exit(-1);
    }
    else
        std::cout << "setsockopt() is OK" << std::endl;

    std::cout << "Using:::::Source IP:" << iph_sourceip << "port: "
            << atoi(tcph_srcport) << ", Target IP: " << iph_destip
            << "port: " << atoi(tcph_destport) << std::endl;

// sendto() loop, send every 2 second for 20 counts
    unsigned int count;
    for(count = 0; count < 20; count++)
    {
        int return_value = sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin));
        if (return_value == ENOTCONN)
// Verify
        {
            std::cout << "socket not connected" << std::endl;
            exit(1);
        } else if (return_value == EISCONN)
        {
            std::cout << "address is ignored" << std::endl;
            exit(1);
        }
        else
            std::cout << "Count #" << count << " - sendto() is OK" << std::endl;
        sleep(1);
    }
    close(sd);
    return 0;
}