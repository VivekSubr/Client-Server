#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>

// Calculates checksum for IP header
unsigned short checksum(unsigned short *buf, int nwords) 
{
    unsigned long sum = 0;
    for (; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return static_cast<unsigned short>(~sum);
}

int main() 
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Enable IP header inclusion
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt(IP_HDRINCL)");
        return 1;
    }

    char packet[4096];
    memset(packet, 0, sizeof(packet));

    // Pointer to IP header
    struct iphdr *ip = (struct iphdr *)packet;
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0b10111000; // DSCP 46 (Expedited Forwarding) in upper 6 bits
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr));
    ip->id = htons(12345);
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = inet_addr("192.168.1.10"); // Source IP
    ip->daddr = inet_addr("192.168.1.20"); // Destination IP
    ip->check = checksum((unsigned short *)ip, ip->ihl << 1);

    // UDP header
    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct iphdr));
    udp->source = htons(1234);
    udp->dest = htons(5678);
    udp->len = htons(sizeof(struct udphdr));
    udp->check = 0; // Optional unless using pseudo-header checksum

    // Destination address setup
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = ip->daddr;

    // Send packet
    if (sendto(sockfd, packet, ntohs(ip->tot_len), 0,
               (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        return 1;
    }

    std::cout << "Raw packet sent successfully!\n";
    close(sockfd);
    return 0;
}