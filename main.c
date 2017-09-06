#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "lib/tp3opt.h"
#include "lib/common.h"
#include "lib/routers.h"

#define PRI_BIN_INT8            "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BIN_INT8(i)     \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRI_BIN_INT16           PRI_BIN_INT8 "." PRI_BIN_INT8
#define BYTE_TO_BIN_INT16(i)    BYTE_TO_BIN_INT8((i) >> 8), BYTE_TO_BIN_INT8(i)
#define PRI_BIN_INT32           PRI_BIN_INT16 "." PRI_BIN_INT16
#define BYTE_TO_BIN_INT32(i)    BYTE_TO_BIN_INT16((i) >> 16), BYTE_TO_BIN_INT16(i)

#define TAB_FORMAT              "%6s"

rs_opt options;
short shutdown_flag;

void sigint_handler(int);
void debug_print_address_data(const char*, struct in_addr);

int main(int argc, char *argv[]) {

    network_topology top;
    packet rand_ip;
    int n;

    signal(SIGINT, sigint_handler);

    // Set options for application
    if (rsopt_set(argc, argv, &options) < 0) {
        fprintf(stderr, "A error occurred. Exiting application.\n");
        return EXIT_FAILURE;
    }
    memset(&top, 0, sizeof(top));
    rs_open(options.router_tables_file, options.interface_info_files, &top);
    if (options.debug_opt)
        rs_debug(&top);
    puts("Press any key to continue and generate first package...");
    getchar();
    for (n = 0; !shutdown_flag; n++) {
        puts(DIV_LINE);
        puts(MINOR_DIV_LINE);
        puts("Generating random IP address...");
        pkt_generate_random_ip(&rand_ip);
        if (options.debug_opt)
            debug_print_address_data("IP", rand_ip.dest_addr);
        puts(MINOR_DIV_LINE);
        puts("Sending packet generate over network...");
        puts(MINOR_DIV_LINE);
        rs_send_packet(rand_ip, &top, 0);
        puts(MINOR_DIV_LINE);
        puts("Press any key to generate another package...");
        puts(MINOR_DIV_LINE);
        getchar();
    }
    puts(DIV_LINE);
    puts("\nExiting application...");
    rs_close(&top);
    sleep(1);
    return 0;
}

void sigint_handler(int signum) {
    shutdown_flag = 1;
}

void debug_print_address_data(const char *label, struct in_addr addr) {
    printf(""TAB_FORMAT" (int): %u\n", label, addr.s_addr);
    printf(""TAB_FORMAT" (bin): "PRI_BIN_INT32 "\n", label,
           BYTE_TO_BIN_INT32(addr.s_addr));
    printf(""TAB_FORMAT" (cod): ", label);
    addr.s_addr = htonl(addr.s_addr); // This is necessary for printing correctly
    printf(inet_ntoa(addr));
    putchar('\n');
}
