//
// Created by darkwolf on 05/09/17.
//

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "packet.h"

void pkt_generate_random_ip(packet *pkt) {
    static int first_time = 1;
    if (first_time) {
        srand((unsigned int) time(NULL)); // Generate random seed (run once)
        first_time = 0;
    }
    static const uint32_t min = 0xC0A80000; // 192.168.0.0
    static const uint32_t max = 0xC0A8FFFF; // 192.168.255.255
    memset(pkt, 0, sizeof(struct in_addr));
    pkt->dest_addr.s_addr = rand() % (max - min + 1) + min;
}

void pkt_generate_netmask(int prefix, packet *pkt) {
    static const uint32_t max = 0xFFFFFFFF;
    memset(pkt, 0, sizeof(packet));
    pkt->dest_addr.s_addr = (prefix <= 0) ? 0 : (max << (ADDR_BITS_SIZE - prefix)) & max;
}
