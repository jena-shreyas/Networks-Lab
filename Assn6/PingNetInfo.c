#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#define BUF_SIZE 1024

// convert hostname to IP address
char *convert_hostname_to_ip(char *host)
{
    struct hostent *hent;
    struct in_addr **addr_list;
    int i;

    if ((hent = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return NULL;
    }

    addr_list = (struct in_addr**)hent->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++)
        return inet_ntoa(*addr_list[i]);

    return NULL;
}

// function to compute checksum
uint16_t checksum(uint16_t *addr, int len) 
{
    int nleft = len;
    uint32_t sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    // Calculate the sum of each 16-bit word
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    // Add in the carry from the last 16-bit word, if any
    if (nleft == 1) {
        *(uint8_t*)(&answer) = *(uint8_t*)w;
        sum += answer;
    }

    // Add in the carries from the sum
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    // Take the one's complement of the sum
    answer = ~sum;

    return answer;
}

void send_latency_packets(){


}

int traceroute(int sockfd, char *ip, int n, int T)
{
    while (1){

        struct ip *iph;
        struct icmp *icmph;
        char buf[BUF_SIZE];

        // Create a zero-data packet (only header, for checking latency)
        iph = (struct ip*)buf;
        icmph = (struct icmp*)(buf + sizeof(struct ip));

        iph->ip_v = 4;
        iph->ip_hl = 5;
        iph->ip_tos = 0;
        iph->ip_len = sizeof(struct ip) + sizeof(struct icmp);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // Input arguments
    char *site = argv[1];
    int n = atoi(argv[2]);
    int T = atoi(argv[3]);

    int sockfd;
    char hostbuffer[256];
    int hostname = gethostname(hostbuffer, sizeof(hostbuffer));

    if (hostname == -1)
    {
        perror("Error while getting hostname!");
        exit(EXIT_FAILURE);
    }

    printf("Local host name : %s\n", hostbuffer);
    char *src_ip = convert_hostname_to_ip(hostbuffer);
    printf("Source IP : %s\n", src_ip);

    // Convert sitename to IP address if it is given
    char *dest_ip = NULL;

    if (inet_aton(site, NULL) == 0)
        dest_ip = convert_hostname_to_ip(site);
    else
        dest_ip = site;
    
    if (dest_ip == NULL)
    {
        fprintf(stderr, "Unable to resolve hostname %s\n", site);
        exit(EXIT_FAILURE);
    }

    printf("Destination IP : %s\n", dest_ip);

    // Create a raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        perror("Error while creating raw socket!");
        exit(EXIT_FAILURE);
    }

    //set socket options
    int opt;
    // setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt));

    struct ip *iph;
    struct icmp *icmph;
    char buf[BUF_SIZE];

    // Create a zero-data packet (only header, for checking latency)
    // iph = (struct ip*)buf;
    // icmph = (struct icmp*)(buf + sizeof(struct ip));
    // size_t size = sizeof(struct ip) + sizeof(struct icmp);

    // // Set IP header fields
    // iph->ip_v = 4;
    // iph->ip_hl = 5;
    // iph->ip_tos = 0;
    // iph->ip_len = size;
    // iph->ip_id = htons(54321);
    // iph->ip_off = 0;
    // iph->ip_ttl = 2;
    // iph->ip_p = IPPROTO_ICMP;
    // iph->ip_src.s_addr = inet_addr(src_ip);
    // iph->ip_dst.s_addr = inet_addr(dest_ip);

    // iph->ip_sum = 0;
    // iph->ip_sum = checksum((uint16_t*)iph, sizeof(struct ip));

    // Set ICMP header fields for ECHO_REQUEST

    icmph = (struct icmp*)buf;
    icmph->icmp_type = ICMP_ECHO;
    icmph->icmp_code = 0;
    icmph->icmp_id = htons(12345);
    icmph->icmp_seq = 0;
    icmph->icmp_cksum = 0;
    icmph->icmp_cksum = checksum((uint16_t*)icmph, sizeof(struct icmp));

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);

    // Send the packet
    printf("Sending ICMP packet ...\n");
    if (sendto(sockfd, buf, sizeof(struct icmp), 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Error while sending packet!");
        exit(EXIT_FAILURE);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    }

    // Clear the buffer
    memset(buf, 0, BUF_SIZE);

    // Receive the packet
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&addr, &addr_len) == -1)
    {
        perror("Error while receiving packet!");
        exit(EXIT_FAILURE);
    }

    // Print the packet
    printf("Packet received from %s\n", inet_ntoa(addr.sin_addr));
    
    // Get the IP header from the packet
    struct ip *ip_header = (struct ip*)buf;

    // Print the IP header fields
    printf("IP Header:\n");
    printf("Version: %d\n", ip_header->ip_v);
    printf("Header Length: %d\n", ip_header->ip_hl);
    printf("Type of Service: %d\n", ip_header->ip_tos);
    printf("Total Length: %d\n", ntohs(ip_header->ip_len));
    printf("Identification: %d\n", ntohs(ip_header->ip_id));
    printf("Fragment Offset: %d\n", ntohs(ip_header->ip_off));
    printf("Time to Live: %d\n", ip_header->ip_ttl);
    printf("Protocol: %d\n", ip_header->ip_p);
    printf("Header Checksum: %d\n", ntohs(ip_header->ip_sum));
    // printf("Source IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->ip_src.));
    // printf("Destination IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));

    struct icmp* icmp_hdr = (struct icmp*) (buf + sizeof(struct ip));
    printf("ICMP type: %d\n", icmp_hdr->icmp_type);
    printf("ICMP code: %d\n", icmp_hdr->icmp_code);
    printf("ICMP checksum: %d\n", icmp_hdr->icmp_cksum);

    // Print the packet payload
    printf("Packet contents:\n");
    printf("%s\n", buf + sizeof(struct ip) + sizeof(struct icmp));

    close(sockfd);
    return 0;

    // // Trace route to the given IP
    // printf("Tracing route to %s...\n\n", ip);

    // if (traceroute(sockfd, ip, n, T) == -1)
    // {
    //     printf("Error while tracing route!\n");
    //     exit(EXIT_FAILURE);
    // }
    // else
    //     printf("Route traced successfully!\n");
}