#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include "lib/tp3opt.h"
#include "lib/common.h"
#include "lib/debug.h"

#define PRI_BIN_INT8 "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRI_BIN_INT16 \
    PRI_BIN_INT8    "."     PRI_BIN_INT8
#define BYTE_TO_BINARY_INT16(i) \
    BYTE_TO_BINARY_INT8((i) >> 8),   BYTE_TO_BINARY_INT8(i)
#define PRI_BIN_INT32 \
    PRI_BIN_INT16   "."     PRI_BIN_INT16
#define BYTE_TO_BINARY_INT32(i) \
    BYTE_TO_BINARY_INT16((i) >> 16), BYTE_TO_BINARY_INT16(i)

#define ADDR_BITS_SIZE  32
#define TAB_FORMAT      "%12s"

rs_opt options;
short shutdown_flag;

void sigint_handler(int);
void generate_random_ip(struct in_addr*);
void generate_netmask(int, struct in_addr*);
void debug_print_address_data(const char*, struct in_addr);

int main(int argc, char *argv[]) {

    struct in_addr rand_ip;
    struct in_addr rand_sm;
    struct in_addr ip_masked;
    int n;

    signal(SIGINT, sigint_handler);
    srand((unsigned int) time(NULL)); // Generate random seed
    puts(DIV_LINE);

    // Set options for application
    if (rsopt_set(argc, argv, &options) < 0) {
        fprintf(stderr, "A error occurred. Exiting application.\n");
        return EXIT_FAILURE;
    }
    puts(DIV_LINE);
    for (n = 0; !shutdown_flag; n++) {
#if DEBUG > 0
        puts(MINOR_DIV_LINE);
        puts("Generating random IP address and netmask:");
        n = (n % (ADDR_BITS_SIZE + 1));
        printf(""TAB_FORMAT" (pfx): /%d\n", "Subnet", n);
        generate_netmask(n, &rand_sm);
        debug_print_address_data("Subnet", rand_sm);
        generate_random_ip(&rand_ip);
        debug_print_address_data("IP", rand_ip);
        ip_masked.s_addr = rand_ip.s_addr & rand_sm.s_addr;
        debug_print_address_data("IP AND Sub", ip_masked);
        puts(MINOR_DIV_LINE);
        puts(DIV_LINE);
#endif
    }
    puts("\nExiting application...");
    sleep(1);
    return 0;
}

void sigint_handler(int signum) {
    shutdown_flag = 1;
}

void generate_random_ip(struct in_addr *ip) {
    static const uint32_t min = 0xC0A80000; // 192.168.0.0
    static const uint32_t max = 0xC0A8FFFF; // 192.168.255.255
    memset(ip, 0, sizeof(struct in_addr));
    ip->s_addr = htonl(rand() % (max - min + 1) + min);
}

void generate_netmask(int prefix, struct in_addr *nm) {
    static const uint32_t max = 0xFFFFFFFF;
    memset(nm, 0, sizeof(struct in_addr));
    nm->s_addr = (prefix == 0) ? 0 : htonl((max << (ADDR_BITS_SIZE - prefix)) & max);
}

void debug_print_address_data(const char *label, struct in_addr addr) {
    printf(""TAB_FORMAT" (int): %u\n", label, addr.s_addr);
    printf(""TAB_FORMAT" (bin): "PRI_BIN_INT32 "\n", label,
           BYTE_TO_BINARY_INT32(htonl(addr.s_addr)));
    printf(""TAB_FORMAT" (cod): ", label);
    printf(inet_ntoa(addr));
    putchar('\n');
}
