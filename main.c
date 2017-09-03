#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include "lib/tp3opt.h"
#include "lib/common.h"

rs_opt options;
short shutdown_flag;

void sigint_handler(int);
void generate_random_ip(struct in_addr*);

int main(int argc, char *argv[]) {

    struct in_addr rand_ip;

    signal(SIGINT, sigint_handler);
    srand((unsigned int) time(NULL)); // Generate random seed
    puts(DIV_LINE);

    // Set options for application
    if (rsopt_set(argc, argv, &options) < 0) {
        fprintf(stderr, "A error occurred. Exiting application.\n");
        return EXIT_FAILURE;
    }
    while (!shutdown_flag) {
        generate_random_ip(&rand_ip);
        puts(inet_ntoa(rand_ip));
    }
    puts("\nExiting application...");
    sleep(1);
    return 0;
}

void sigint_handler(int signum) {
    shutdown_flag = 1;
}

void generate_random_ip(struct in_addr *ip) {
    memset(ip, 0, sizeof(struct in_addr));
    ip->s_addr = (in_addr_t) rand();
}
