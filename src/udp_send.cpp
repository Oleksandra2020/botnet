#include "../inc/udp_send.h"

// total udp header length: 8 bytes (=64 bits)

// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short *buf, int nwords)
{       //
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}


// Source IP, source port, target IP, target port from the command line arguments
//int main(int argc, char *argv[])
int send_udp(const char* iph_sourceip, const char* udph_srcport, const char* iph_destip, const char* udph_destport, int argc)
{
    int sd;
// No data/payload just datagram
    char buffer[PCKT_LEN];
// Our own headers' structures
    auto *ip = (struct ipheader *) buffer;
    auto *udp = (struct udpheader *) (buffer + sizeof(struct ipheader));
// Source and destination addresses: IP and port
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

// Create a raw socket with UDP protocol
    sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0)
    {
        perror("socket() error");
// If something wrong just exit
        exit(-1);
    }
    else
        std::cout << "socket() - Using SOCK_RAW socket and UDP protocol is OK." << std::endl;

// The source is redundant, may be used later if needed
// The address family
    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;
// Port numbers
    sin.sin_port = htons(atoi(udph_srcport));
    din.sin_port = htons(atoi(udph_destport));
// IP addresses
    sin.sin_addr.s_addr = inet_addr(iph_sourceip);
    din.sin_addr.s_addr = inet_addr(iph_destip);

// Fabricate the IP header or we can use the
// standard header structures but assign our own values.
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 16; // Low delay
    ip->iph_len = sizeof(struct ipheader) + sizeof(struct udpheader);
    ip->iph_ident = htons(54321);
    ip->iph_ttl = 64; // hops
    ip->iph_protocol = 17; // UDP
// Source IP address, can use spoofed address here!!!
    ip->iph_sourceip = inet_addr(iph_sourceip);
// The destination IP address
    ip->iph_destip = inet_addr(iph_destip);

// Fabricate the UDP header. Source port number, redundant
    udp->udph_srcport = htons(atoi(udph_srcport));
// Destination port number
    udp->udph_destport = htons(atoi(udph_destport));
    udp->udph_len = htons(sizeof(struct udpheader));
// Calculate the checksum for integrity
    ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
// Inform the kernel do not fill up the packet structure. we will build our own...

// !!!!!!!!!!!!!!!!!
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
//    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR , val, sizeof(one)))
    {
        perror("setsockopt() error");
        exit(-1);
    }
    else
        std::cout << "setsockopt() is OK" << std::endl;

// Send loop, send for every 2 second for 100 count
    std::cout << "Trying..." << std::endl;
    std::cout << "Using raw socket and UDP protocol" << std::endl;
    std::cout << "Using Source IP: " << iph_sourceip << "port: " << atoi(udph_srcport)
              << ", Target IP: " << iph_destip << " port: ." <<  atoi(udph_destport) << std::endl;

    int count;
    for(count = 1; count <=20; count++)
    {
        if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
// Verify
        {
            perror("sendto() error");
            exit(-1);
        }
        else
        {
            std::cout << "Count #" << count << " - sendto() is OK" << std::endl;
            sleep(2);
        }
    }
    close(sd);
    return 0;
}
