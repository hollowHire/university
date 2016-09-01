#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
//#include <netpacket/packet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct arpbuf{
    struct ether_header eth;
    struct ether_arp arp;
};

void sendarp(char *eth_src_mac,char *eth_dst_mac,char *arp_src_mac,
             char *arp_dst_mac,char *src_ip,char *dst_ip,char *ifname,int op){
    int buflen = sizeof(arpbuf);
    char buf[buflen];
    struct arpbuf *abuf=(struct arpbuf *)buf;
    struct sockaddr_ll toaddr;
    struct in_addr targetIP,srcIP;
    struct ifreq ifr;
    int skfd;
    
    if((skfd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP))) < 0){
        exit(1);
    }
    
    bzero(&toaddr,sizeof(toaddr));
    bzero(&ifr,sizeof(ifr));
    
    memcpy(ifr.ifr_name,ifname,strlen(ifname));
    
    ioctl(skfd,SIOCGIFINDEX,&ifr);
    
    toaddr.sll_ifindex = ifr.ifr_ifindex;
    
    memcpy(abuf->eth.ether_dhost,eth_dst_mac,ETH_ALEN);
    memcpy(abuf->eth.ether_shost,eth_src_mac,ETH_ALEN);
    abuf->eth.ether_type = htons(ETHERTYPE_ARP);
    
    abuf->arp.arp_hrd = htons(ARPHRD_ETHER);
    abuf->arp.arp_pro = htons(ETHERTYPE_IP);
    
    abuf->arp.arp_hln = ETH_ALEN;
    abuf->arp.arp_pln = 4;
    
    abuf->arp.arp_op = htons(op == 1 ? ARPOP_REQUEST : ARPOP_REPLY);
    
    memcpy(abuf->arp.arp_sha,arp_src_mac,ETH_ALEN);
    
    inet_pton(AF_INET,src_ip,srcIP);
    
    memcpy(abuf->arp.arp_spa,&srcIP,4);
    
    memcpy(abuf->arp.arp_tha,arp_dst_mac,ETH_ALEN);
    inet_pton(AF_INET,dst_ip,&targetIP);
    memcpy(abuf->arp.arp_tpa,&targetIP,4);
    
    toaddr.sll_family = PF_PACKET;
    
    sendto(skfd,buf,buflen,0,(struct sockaddr *)&toaddr,sizeof(toaddr));
    
    close(skfd);
    
}

