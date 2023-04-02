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
#include <time.h>
#define BUF_SIZE 1024

int ttl = 1;

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

int find_node_compute_latency(int sockfd, char *dest_ip, char *node_ip)
{
    srand(time(NULL));

    int n = 5;
    int id, seq_no = 0;
    char dest[50];
    strcpy(dest, dest_ip);

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest);
    printf("\n\nDestination IP inside latency fn : %s\n", dest_ip); 
    printf("\n\nTTL value: %d\n\n", ttl);

    while (n--)
    {
        // Set TTL value
        printf("n : %d\n", n);
        id = rand() % 30000 + 1;
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        struct ip *iph;
        struct icmp *icmph;
        char buf[BUF_SIZE];

        // Create a zero-data packet (only header, for checking latency)
        icmph = (struct icmp*)buf;
        size_t size = sizeof(struct ip) + sizeof(struct icmp);

        icmph->icmp_type = ICMP_ECHO;
        icmph->icmp_code = 0;
        icmph->icmp_id = htons(id);
        icmph->icmp_seq = seq_no;
        icmph->icmp_cksum = 0;
        icmph->icmp_cksum = htons(checksum((uint16_t*)icmph, sizeof(struct icmp)));

        // // Send the packet

        // printf("\n\n**** SENT PACKET DETAILS ****\n\n");

        // printf("\nICMP Header :\n\n");
        // printf("Type: %d\n", icmph->icmp_type);
        // printf("Code: %d\n", icmph->icmp_code);
        // printf("Identifier: %d\n", ntohs(icmph->icmp_id));
        // printf("Sequence Number: %d\n", icmph->icmp_seq);
        // printf("Checksum: %d\n", ntohs(icmph->icmp_cksum));

        // printf("\n\nSending ICMP packet ...\n\n");

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

        printf("Waiting for packet ...\n");

        if (recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&addr, &addr_len) == -1)
        {
            perror("Error while receiving packet!");
            exit(EXIT_FAILURE);
        }

        // // Print the packet
        // printf("Packet received from %s\n", inet_ntoa(addr.sin_addr));
        
        // Get the IP header from the packet
        struct ip *ip_header = (struct ip*)buf;

        printf("DIP : %s\n", dest);

        // // Print the IP header fields
        // printf("\n\n**** RECEIVED PACKET DETAILS ****\n\n");

        // printf("IP Header :\n\n");
        // printf("Version: %d\n", ip_header->ip_v);
        // printf("Header Length: %d\n", ip_header->ip_hl);
        // printf("Type of Service: %d\n", ip_header->ip_tos);
        // printf("Total Length: %d\n", ntohs(ip_header->ip_len));
        // printf("Identification: %d\n", ntohs(ip_header->ip_id));
        // printf("Fragment Offset: %d\n", ntohs(ip_header->ip_off));
        // printf("Time to Live: %d\n", ntohs(ip_header->ip_ttl));
        // printf("Protocol: %d\n", ip_header->ip_p);
        // printf("Header Checksum: %d\n", ntohs(ip_header->ip_sum));
        // printf("Source IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->ip_src.s_addr));
        // printf("Destination IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->ip_dst.s_addr));

        strcpy(node_ip, inet_ntoa(*(struct in_addr *)&ip_header->ip_src.s_addr));

        // struct icmp* icmp_hdr = (struct icmp*) (buf + sizeof(struct ip));
        // printf("\nICMP Header :\n\n");
        // printf("ICMP type: %d\n", icmp_hdr->icmp_type);
        // printf("ICMP code: %d\n", icmp_hdr->icmp_code);
        // printf("ICMP checksum: %d\n", ntohs(icmp_hdr->icmp_cksum));
        // printf("ICMP id: %d\n", ntohs(icmp_hdr->icmp_id));

        // // Print the packet payload
        // printf("Packet contents:\n");
        // printf("%s\n", buf + sizeof(struct ip) + sizeof(struct icmp));

        printf("Packet with sequence no %d received from %s\n", seq_no, node_ip);
        seq_no++;
        printf("Sleeping for 1 second ...\n\n");
        sleep(1);
    }   

    ttl++;
    printf("Destination IP before leaving latency fn : %s\n\n\n", dest);
    return 0;
}

int compute_bandwidth(int sockfd, char *node_ip, int n, int T)
{
    // Send n packets to node_ip and find the average bandwidth
    return 0;
}

int traceroute(int sockfd, char *src_ip, char *dest_ip, int n, int T)
{
    char node_ip[50], prev_node_ip[50], src[50], dest[50];
    printf("Source IP in traceroute : %s\n", src_ip);
    printf("Destination IP in traceroute : %s\n", dest_ip);
    strcpy(src, src_ip);
    strcpy(dest, dest_ip);
    strcpy(prev_node_ip, src_ip);

    while (1){

        strcpy(dest_ip, dest);
        int latency = find_node_compute_latency(sockfd, dest_ip, node_ip);
        printf("Latency of link between %s and %s: %d\n", prev_node_ip, node_ip, latency);

        int bandwidth = compute_bandwidth(sockfd, node_ip, n, T);
        printf("Bandwidth of link between %s and %s: %d\n", prev_node_ip, node_ip, bandwidth);

        printf("NODE IP : %s\n", node_ip);
        printf("DEST IP : %s\n", dest);

        if (strcmp(node_ip, dest) == 0)
        {
            printf("Destination reached!\n");
            break;
        }
        
        strcpy(prev_node_ip, node_ip);
    }
    return -1;
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

    char src_ip[50];
    strcpy(src_ip, convert_hostname_to_ip(hostbuffer));
    printf("Source IP : %s\n", src_ip);

    // Convert sitename to IP address if it is given
    char dest_ip[50];

    if (inet_aton(site, NULL) == 0)
        strcpy(dest_ip, convert_hostname_to_ip(site));
    else
        strcpy(dest_ip, site);

    printf("Destination IP : %s\n", dest_ip);

    // Create a raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        perror("Error while creating raw socket!");
        exit(EXIT_FAILURE);
    }

    // Trace route to destination
    printf("SRC IP IN MAIN : %s\n", src_ip);
    printf("DEST IP IN MAIN : %s\n", dest_ip);
    // traceroute(sockfd, src_ip, dest_ip, n, T);

    //set ttl value
    int ttl = 5;
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    struct ip *iph;
    struct icmphdr *icmph;
    char buf[BUF_SIZE];

    // Create a zero-data packet (only header, for checking latency)
    // iph = (struct ip*)buf;
    icmph = (struct icmphdr*)buf;
    // size_t size = sizeof(struct ip) + sizeof(struct icmp);

    // // Set IP header fields
    // iph->ip_v = 4;
    // iph->ip_hl = 5;
    // iph->ip_tos = 0;
    // iph->ip_len = size;
    // iph->ip_id = htons(54321);
    // iph->ip_off = 0;
    // iph->ip_ttl = 200;
    // iph->ip_p = IPPROTO_ICMP;
    // iph->ip_src.s_addr = inet_addr(src_ip);
    // iph->ip_dst.s_addr = inet_addr(dest_ip);
    // iph->ip_sum = 0;
    // iph->ip_sum = htons(checksum((uint16_t*)iph, sizeof(struct ip)));

    // Set ICMP header fields for ECHO_REQUEST

    // icmph = (struct icmp*)buf;
    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    // icmph->icmp_id = htons(12345);
    // icmph->icmp_seq = 0;
    icmph->checksum = 0;
    icmph->checksum = checksum((uint16_t*)icmph, sizeof(struct icmphdr));

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);

    // Send the packet

    printf("\n\n**** SENT PACKET DETAILS ****\n\n");

    printf("IP Header :\n\n");
    printf("Version: %d\n", iph->ip_v);
    printf("Header Length: %d\n", iph->ip_hl);
    printf("Type of Service: %d\n", iph->ip_tos);
    printf("Total Length: %d\n", iph->ip_len);
    printf("Identification: %d\n", ntohs(iph->ip_id));
    printf("Fragment Offset: %d\n", ntohs(iph->ip_off));
    printf("Time to Live: %d\n", iph->ip_ttl);
    printf("Protocol: %d\n", iph->ip_p);
    printf("Header Checksum: %d\n", ntohs(iph->ip_sum));

    printf("\nICMP Header :\n\n");
    printf("Type: %d\n", icmph->type);
    printf("Code: %d\n", icmph->code);
    // printf("Identifier: %d\n", ntohs(icmph->icmp_id));
    // printf("Sequence Number: %d\n", ntohs(icmph->icmp_seq));
    printf("Checksum: %d\n", ntohs(icmph->checksum));

    printf("\n\nSending ICMP packet ...\n\n");
    if (sendto(sockfd, buf, sizeof(struct icmphdr), 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1)
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
    printf("**** RECEIVED PACKET DETAILS ****\n\n");

    printf("IP Header :\n\n");
    printf("Version: %d\n", ip_header->ip_v);
    printf("Header Length: %d\n", ip_header->ip_hl);
    printf("Type of Service: %d\n", ip_header->ip_tos);
    printf("Total Length: %d\n", ntohs(ip_header->ip_len));
    printf("Identification: %d\n", ntohs(ip_header->ip_id));
    printf("Fragment Offset: %d\n", ntohs(ip_header->ip_off));
    printf("Time to Live: %d\n", ntohs(ip_header->ip_ttl));
    printf("Protocol: %d\n", ip_header->ip_p);
    printf("Header Checksum: %d\n", ntohs(ip_header->ip_sum));
    printf("Source IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->ip_src.s_addr));
    printf("Destination IP Address: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->ip_dst.s_addr));

    struct icmphdr* icmp_hdr = (struct icmphdr*) (buf + sizeof(struct ip));
    printf("\nICMP Header :\n\n");
    printf("ICMP type: %d\n", icmp_hdr->type);
    printf("ICMP code: %d\n", icmp_hdr->code);
    printf("ICMP checksum: %d\n", ntohs(icmp_hdr->checksum));
    // printf("ICMP id: %d\n", ntohs(icmp_hdr->icmp_id));

    // Print the packet payload
    printf("Packet contents:\n");
    printf("%s\n", buf + sizeof(struct ip) + sizeof(struct icmphdr));

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