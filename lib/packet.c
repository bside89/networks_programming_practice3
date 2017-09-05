//
// Created by darkwolf on 05/09/17.
//

#include <string.h>
#include <stdint.h>
#include "packet.h"

void pkt_generate_netmask(int prefix, struct in_addr *nm) {
    static const uint32_t max = 0xFFFFFFFF;
    memset(nm, 0, sizeof(struct in_addr));
    nm->s_addr = (prefix <= 0) ? 0 : (max << (ADDR_BITS_SIZE - prefix)) & max;
}
