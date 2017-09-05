#ifndef TP3_PACKET_H
#define TP3_PACKET_H

#include <arpa/inet.h>

#define ADDR_BITS_SIZE 32

void pkt_generate_netmask(int prefix, struct in_addr *nm);

#endif //TP3_PACKET_H
