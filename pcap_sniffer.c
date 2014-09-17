#include <stdio.h>
#include <pcap.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

struct sniff_icmp {
	u_char type;			/* ICMP type */
	u_char code;			/* ICMP code */
	u_short checksum;		/* Checksum of ICMP header and data */
	union {
    		struct {
      			u_int16_t id;
      			u_int16_t sequence;
    		} echo;			/* echo datagram */
    		u_int32_t gateway;	/* gateway address */
    		struct {
      			u_int16_t __unused;
      			u_int16_t mtu;
    		} frag;			/* path mtu discovery */
  	} un;
};

void print_hex_ascii_line(const u_char *payload, int len, int offset)
{

	int i;
	int gap;
	const u_char *ch;

	/* offset */
	printf("%05d   ", offset);
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");

	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

	return;
}

void print_payload(const u_char *payload, int len)
{

	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}

	return;
}

/* callback function for the filter.
 * uchar *args : Private data from the main program.
 * pcap_pkthdr : pcap header of the sniffed packet.
 *		 struct pcap_pkthdr {
 *		 	struct timeval ts; // time stamp 
 *			bpf_u_int32 caplen; // length of portion present 
 *			bpf_u_int32 len; // length this packet (off wire) 
 *		 };
 * ucar* packet : the first byte of a chunk of data containing the entire packet
 * 		  sniffed by pcap_loop
 */
void packet_callback (u_char *args, const struct pcap_pkthdr *header, 
		const u_char *packet)
{
	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */
	const struct sniff_tcp *tcp; /* The TCP header */
	const struct sniff_icmp *icmp; /* ICMP header */
	const char *payload; /* Packet payload */

	u_int size_ip;
	u_int size_tcp;

	printf("\n****************************************************************************\n");
	/* Typecasting the ethernet frame */
	ethernet = (struct sniff_ethernet*)(packet);

	/* Getting the IP structure pointer by adding ETHERNET  size to the packet pointer */
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);

	/* IP_HL returns the IP header length and IP_V returns the IP version number */
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}

	/* print source and destination IP addresses */
	printf("GOT A PACKET of length %u\n", ip->ip_len);
	printf("	From: %s\n", inet_ntoa(ip->ip_src));
	printf("	To: %s\n", inet_ntoa(ip->ip_dst));

	if (ip->ip_p == 1) {
		/*ICMP PACKET*/
		size_tcp = sizeof(struct sniff_icmp);
		printf("	ICMP header of length: %u\n", size_tcp);
		icmp = (struct sniff_icmp *)(packet + SIZE_ETHERNET + size_ip);
		switch (icmp->type) {
			case 0  : 	printf("	ICMP TYPE : ECHO REPLY\n");
					break;
			case 3  :	printf("	ICMP TYPE : DESTINATION UNREACHABLE\n");
					break;
			case 8  : 	printf("	ICMP TYPE : ECHO REQUEST\n");
					break;
			case 30 :	printf(" 	ICMP TYPE : TRACE ROUTE\n");
					break;
			default	:	printf("	ICMP TYPE : UNKNOWN\n");
					break;
		}
	} else {
		tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
		size_tcp = TH_OFF(tcp)*4;
		if (size_tcp < 20) {
			printf("	* Invalid TCP header length: %u bytes *\n", size_tcp);
		} else {
			printf("   	Src port: %d\n", ntohs(tcp->th_sport));
			printf("	Dst port: %d\n", ntohs(tcp->th_dport));
		}
	}

	payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
	/* compute tcp payload (segment) size */
	int size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
	/*
	 * Print payload data; it might be binary, so don't just
	 * treat it as a string.
	 */
	if (size_payload > 0) {
		printf("   Payload (%d bytes):\n", size_payload);
		print_payload(payload, size_payload);
	}

	printf("\n****************************************************************************\n");

	return;
}

int main(int argc, char *argv[])
{
	/* PCAP_ERRBUF_SIZE is defined in pcap.h */
	char errbuf[PCAP_ERRBUF_SIZE], *somedev = argv[1];
	pcap_t *handle;
	struct bpf_program fp;			/* The compiled filter */
	char filter_exp[] = "ip proto 1";	/* The filter expression */
	bpf_u_int32 mask;			/* Our netmask */
	bpf_u_int32 net;			/* Our IP */
	struct pcap_pkthdr header;		/* The header that pcap gives us */
	const u_char *packet;			/* The actual packet */
	struct pcap_stat ps;			/* Packet capture statistics*/

	printf("Device: %s : Error Buffer %d : Snapshot length %d\n", somedev, PCAP_ERRBUF_SIZE, BUFSIZ);
	
	/* Open a device somedev (all devices if somedev is NULL or any), with
	 * snaplen set to BUFSIZ (defined in pcap.h), promiscuous mode set to 1,
	 * with read timeout set to 1000ms. Error saved in errbuf
	 */
	handle = pcap_open_live(somedev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		pcap_perror(handle, "pcap_open_live");
		return(2);
	}
	if (pcap_datalink(handle) != DLT_EN10MB) {
		pcap_perror(handle, "pcap_datalink");
		return(2);
	}
	/* Compile and apply the filter. Given an interface handle and
	 * filter_exp string, the string is compiled into a filter program of
	 * type struct bfp_program. optimize flag is set to zero and last
	 * argument is the netmask
	 */
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		pcap_perror(handle, "pcap_compile");
		return(2);
	}
	/* Set the new filter program on the interface handle */
	if (pcap_setfilter(handle, &fp) == -1) {
		pcap_perror(handle, "pcap_setfilter");
		return(2);
	}

	if (0) {
		/* Grab a packet */
		packet = pcap_next(handle, &header);
		/* Print its length */
		printf("Jacked a packet with length of [%d]\n", header.len);
	}
	/* loop on the interface handle 10 times calling the packet_callback
	 * function everytime we get a packet that matches the filter. The last
	 * argument is private data that you might want to send the callback
	 * function.
	 */
	if (pcap_loop(handle, 10, packet_callback, NULL) == -1) {
		pcap_perror(handle, "pcap_loop");
		return(2);
	}

	/* Collect statistics */
	if (pcap_stats(handle, &ps) == -1) {
		pcap_perror(handle, NULL);
		return(2);
	} else {
		printf("\n\n***  PACKETS RECEIVED 			: 	%d\n", ps.ps_recv);
		printf("***  PACKETS DROPPED BY OS BUFFER	:	%d\n", ps.ps_drop);
	}
		
	/* And close the session */
	pcap_close(handle);

	return(0);
}
