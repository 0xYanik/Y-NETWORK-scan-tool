#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
struct icmphdr icmp_struct;
struct sockaddr_in dest;
struct iphdr *icmp_cast_iphdr;
uint16_t checksum(void *b, int len) {
    uint16_t *buf = b;
    uint32_t sum = 0;
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

void init_icmp_hdr(){

icmp_struct.type = ICMP_ECHO;
icmp_struct.code = 0;
icmp_struct.un.echo.id = htons(getpid() & 0xFFFF);
icmp_struct.un.echo.sequence = htons(1);
icmp_struct.checksum = 0;
icmp_struct.checksum = checksum(&icmp_struct,sizeof(icmp_struct));


}

int main(int argc,char *argv[])
{
    
    char buffer[1000];

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    // lets put the addr ip entered as an argument
   if (argc >= 3 && strcmp(argv[1], "-t") == 0) {
    if (inet_pton(AF_INET, argv[2], &dest.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", argv[2]);
        return 1;
    }
} else {
    fprintf(stderr, "Usage: %s -t <target_ip>\n", argv[0]);
    return 1;
}

    
    
    memset(&icmp_struct,0,sizeof(icmp_struct));
    init_icmp_hdr();
    int icmp_socket = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    ssize_t icmp_send = sendto(icmp_socket,&icmp_struct,sizeof(icmp_struct),0,
                        (struct sockaddr*)&dest,sizeof(dest));
    socklen_t dest_len = sizeof(dest);
    uint16_t my_id = getpid() & 0xFFFF;
    while(1){
    ssize_t response =  recvfrom(icmp_socket,buffer,sizeof(buffer),0,(struct sockaddr *)&dest,
                        &dest_len);
    
    icmp_cast_iphdr = (struct iphdr*)(buffer);
    struct icmphdr *icmp = (struct icmphdr*)(buffer + icmp_cast_iphdr->ihl *4);
            
       if (icmp->type == ICMP_ECHOREPLY && ntohs(icmp->un.echo.id) == my_id) {
        printf("Status: Ping Reply from %s\n", inet_ntoa(dest.sin_addr));
        printf("ID: %u | Sequence: %u | TTL: %d\n",
               ntohs(icmp->un.echo.id),
               ntohs(icmp->un.echo.sequence),
               icmp_cast_iphdr->ttl);
        break;
    }};



    return 0;
}