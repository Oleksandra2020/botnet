//---cat rawtcp.c---
// Run as root or SUID 0, just datagram no data/payload
#include "packet_sending.h"

int packet_sending::send_tcp(const char *iph_sourceip, const char *tcph_srcport, const char *iph_destip,
			     const char *tcph_destport) {
	int sd;
	// No data, just datagram
	char buffer[PCKT_LEN];
	// The size of the headers
	auto *ip = (struct ipheader *)buffer;
	auto *tcp = (struct tcpheader *)(buffer + sizeof(struct ipheader));
	struct sockaddr_in sin {
	}, din{};
	int one = 1;
	const int *val = &one;

	memset(buffer, 0, PCKT_LEN);

	sd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sd < 0) {
		perror("socket() error");
		exit(-1);
	}

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
	ip->iph_protocol = IPPROTO_TCP;	 // TCP
	ip->iph_chksum = 0;		 // Done by kernel

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
	tcp->tcph_chksum = 0;  // Done by kernel
	tcp->tcph_urgptr = 0;
	// IP checksum calculation
	ip->iph_chksum = csum((unsigned short *)buffer, (sizeof(struct ipheader) + sizeof(struct tcpheader)));

	// Inform the kernel do not fill up the headers' structure, we fabricated our own

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, val, sizeof(one))) {
		perror("setsockopt() error");
		exit(-1);
	}

	int return_value = sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin));
	if (return_value < 0) {
		char buffer[256];
		strerror_r(errno, buffer, 256);
		std::cout << buffer << std::endl;
	} else {
		std::cout << iph_destip << " - sendto() tcp is OK " << std::endl;
	}
	sleep(1);
	close(sd);
	return 0;
}

void packet_sending::send_get_request(const char *host_name) {
	int sock;
	struct sockaddr_in client {};

	struct hostent *host = gethostbyname(host_name);

	if ((host == nullptr) || (host->h_addr == nullptr)) {
		std::cout << "Error retrieving DNS information." << std::endl;
		exit(1);
	}

	bzero(&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	memcpy(&client.sin_addr, host->h_addr, host->h_length);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		std::cout << "Error creating socket." << std::endl;
		exit(1);
	}

	if (connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0) {
		close(sock);
		std::cout << "Could not connect" << std::endl;
		exit(1);
	}

	std::stringstream ss;
	ss << "GET /index.html" << 550 << "HTTP/1.0\r\n"
	   << "Host:" << host_name << "\r\n"
	   << "Accept: application/json\r\n"
	   << "\r\n\r\n";
	std::string request = ss.str();

	if (send(sock, request.c_str(), request.length(), 0) != (int)request.length()) {
		std::cout << "Error sending request." << std::endl;
		exit(1);
	} else {
		std::cout << host_name << " - send() http is OK " << std::endl;
	}

	//    char cur;
	//    while ( read(sock, &cur, 1) > 0 ) {
	//        std::cout << cur;
	//    }
}

unsigned short packet_sending::csum(unsigned short *buf, int len) {
	unsigned long sum;
	for (sum = 0; len > 0; len--) sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}