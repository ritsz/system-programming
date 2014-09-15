#include <stdio.h>
#include <pcap.h>

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


	printf("Device: %s : Error Buffer %d : Snapshot length %d\n", somedev, PCAP_ERRBUF_SIZE, BUFSIZ);
	
	/* Open a device somedev (all devices if somedev is NULL or any), with
	 * snaplen set to BUFSIZ (defined in pcap.h), promiscuous mode set to 1,
	 * with read timeout set to 1000ms. Error saved in errbuf
	 */
	handle = pcap_open_live(somedev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", somedev, errbuf);
		return(2);
	}
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", somedev);
		return(2);
	}
	/* Compile and apply the filter. Given an interface handle and
	 * filter_exp string, the string is compiled into a filter program of
	 * type struct bfp_program. optimize flag is set to zero and last
	 * argument is the netmask
	 */
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	/* Set the new filter program on the interface handle */
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	/* Grab a packet */
	packet = pcap_next(handle, &header);
	/* Print its length */
	printf("Jacked a packet with length of [%d]\n", header.len);
	/* And close the session */
	pcap_close(handle);

	return(0);
}
