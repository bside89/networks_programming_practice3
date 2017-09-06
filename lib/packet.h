#ifndef TP3_PACKET_H
#define TP3_PACKET_H

#include <arpa/inet.h>

#define ADDR_BITS_SIZE 32

typedef struct {
    struct in_addr dest_addr;
} packet;

void pkt_generate_random_ip(packet *pkt);

void pkt_generate_netmask(int prefix, packet *pkt);

#endif //TP3_PACKET_H
