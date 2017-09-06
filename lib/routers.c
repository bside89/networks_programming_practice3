#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "routers.h"
#include "common.h"
#include "packet.h"
#include "debug.h"

#define REGISTRY_SIZE 60

int rs_open(const char *routing_text, const char *interfaces_text,
            network_topology *data) {

    FILE *r_file, *i_file;

    r_file = fopen(routing_text, "r");
    if (r_file == NULL)
        return -1;
    i_file = fopen(interfaces_text, "r");
    if (i_file == NULL)
        return -1;
    rs_get_routing_tables_data(r_file, data);
    rs_get_interface_tables_data(i_file, data);

    fclose(r_file);
    fclose(i_file);
}

void rs_get_routing_tables_data(FILE *source, network_topology *data) {
    int i, j;
    router *r;
    routers_table_entry *re;
    packet pkt;
    char buffer_line[REGISTRY_SIZE];
    char ip_buffer[IP_ADDRESS_SIZE];
    char gw_buffer[IP_ADDRESS_SIZE];

    fgets(buffer_line, REGISTRY_SIZE, source);
    sscanf(buffer_line, "%zu", &data->size);
    for (i = 0; i < data->size; i++) { // For each router
        r = &data->routers[i];
        // Define name
        fgets(buffer_line, REGISTRY_SIZE, source);
        sscanf(buffer_line, "%s", r->name);

        fgets(buffer_line, REGISTRY_SIZE, source);
        sscanf(buffer_line, "%zu", &r->rt.size); // Define routing's table size

        // Alloc table memory
        r->rt.table = calloc(r->rt.size, sizeof(routers_table_entry));
        for (j = 0; j < r->rt.size; j++) { // For each line in routing's table
            re = &r->rt.table[j];
            fgets(buffer_line, REGISTRY_SIZE, source);
            sscanf(buffer_line, "%s %d %s %s",
                   ip_buffer,
                   &re->subnet_mask_prefix,
                   gw_buffer,
                   re->interface_name);
            // Define IP (registry)
            inet_aton(ip_buffer, &re->ip);
            re->ip.s_addr = htonl(re->ip.s_addr);

            // Define subnet mask  (registry)
            pkt_generate_netmask(re->subnet_mask_prefix, &pkt);
            re->subnet_mask = pkt.dest_addr;

            // Define gateway IP (next hop) (registry)
            inet_aton(gw_buffer, &re->gateway);
            re->gateway.s_addr = htonl(re->gateway.s_addr);
        }
    }
}

void rs_get_interface_tables_data(FILE *source, network_topology *data) {
    int i, j;
    router *r;
    interfaces_table_entry *ie;
    packet pkt;
    char buffer_line[REGISTRY_SIZE];
    char name[INTERFACE_NAME_MAX_LENGTH];
    char ip_buffer[IP_ADDRESS_SIZE];

    fgets(buffer_line, REGISTRY_SIZE, source);
    sscanf(buffer_line, "%zu", &data->size);
    for (i = 0; i < data->size; i++) { // For each router
        r = &data->routers[i];

        fgets(buffer_line, REGISTRY_SIZE, source);
        sscanf(buffer_line, "%zu", &r->it.size); // Define routing's table size

        // Alloc table memory
        r->it.table = calloc(r->it.size, sizeof(interfaces_table_entry));
        for (j = 0; j < r->it.size; j++) { // For each line in routing's table
            ie = &r->it.table[j];
            fgets(buffer_line, REGISTRY_SIZE, source);
            sscanf(buffer_line, "%s %d %s",
                   name,
                   &ie->subnet_mask_prefix,
                   ip_buffer);
            // Define name
            strcpy(ie->interface_name, name);

            // Define subnet mask (registry)
            pkt_generate_netmask(ie->subnet_mask_prefix, &pkt);
            ie->subnet_mask = pkt.dest_addr;

            // Define IP (registry)
            inet_aton(ip_buffer, &ie->ip);
            ie->ip.s_addr = htonl(ie->ip.s_addr);
        }
    }
}

void rs_send_packet(packet p, network_topology *top, int routerIndex) {
    int i, j;
    router *r;
    routers_table_entry *re;
    routers_table_entry *re_better_result = NULL;
    interfaces_table_entry *ie;
    interfaces_table_entry *ie_better_result = NULL;
    uint32_t result;
    struct in_addr *next_hop;
    struct in_addr helper; // Dummy var to show correct byte order to user
#if PROJECT_DEBUG > 1
    char ip_debug[IP_ADDRESS_SIZE];
#endif
    char ip_buffer1[IP_ADDRESS_SIZE];
    char ip_buffer2[IP_ADDRESS_SIZE];

    if (!(top->size > 0 && routerIndex >= 0 && routerIndex < top->size))
        return;
    r = &top->routers[routerIndex];
    helper = p.dest_addr;
    helper.s_addr = htonl(helper.s_addr);
    sprintf(ip_buffer1, "%s", inet_ntoa(helper));
    // Verify the interface's table for 'r'
    for (i = 0; i < r->it.size; i++) {
        ie = &r->it.table[i];
#if PROJECT_DEBUG > 1
        printf("Verifying interfaces <%d, for %d>...\n", i, routerIndex);
        printf("IP (from %s):", ie->interface_name);
        helper = ie->ip;
        helper.s_addr = htonl(helper.s_addr);
        puts(inet_ntoa(helper));
#endif
        result = p.dest_addr.s_addr & ie->subnet_mask.s_addr;
        if (result == ie->ip.s_addr) {
            // Long prefix match
            if (!ie_better_result || ie_better_result->subnet_mask_prefix < ie->subnet_mask_prefix) {
                ie_better_result = ie; // Stores the longer prefix 'ie'
            }
        }
    }
    if (ie_better_result) { // Found a match with LPM
        // Ex: r2 is forwarding packet for 1.2.3.4 over eth0 interface
        printf("%s is forwarding packet for %s over %s interface.\n",
               r->name, ip_buffer1, ie_better_result->interface_name);
        return; // Send over to the interface and close connection
    }
    // Verify, now, the routing's table for 'r'
    for (i = 0; i < r->rt.size; i++) {
#if PROJECT_DEBUG > 1
        printf("Verifying routes <%d>...\n", i);
#endif
        re = &r->rt.table[i];
        result = p.dest_addr.s_addr & re->subnet_mask.s_addr;
        if (result == re->ip.s_addr) { // If matched, send to the next hop
            // Long prefix match
            if (!re_better_result ||
                re_better_result->subnet_mask_prefix < re->subnet_mask_prefix) {
                re_better_result = re; // Stores the longer prefix 're'
            }
        }
    }
    if (re_better_result) { // Found a match with LPM
        next_hop = &re_better_result->gateway;
#if PROJECT_DEBUG
        puts("Addresses matched. Searching next hop now...");
        printf("Address (packet): ");
        helper = re_better_result->ip;
        helper.s_addr = htonl(helper.s_addr);
        puts(inet_ntoa(helper));
        printf("And the gateway is: ");
        helper = *next_hop;
        helper.s_addr = htonl(helper.s_addr);
        puts(inet_ntoa(helper));
#endif
        // Verify all interface's table to see who is the next router in hop
        for (i = 0; i < top->size; i++) {
            if (i == routerIndex)
                continue;
#if PROJECT_DEBUG > 1
            printf("Verifying interfaces for determine next hop <%d>...\n", j);
#endif
            for (j = 0; j < top->routers[i].it.size; j++) {
                ie = &top->routers[i].it.table[j];
#if PROJECT_DEBUG > 1
                printf("IP (from %s):", ie->interface_name);
                helper = ie->ip;
                helper.s_addr = htonl(helper.s_addr);
                puts(inet_ntoa(helper));
#endif
                // If address is equal to the next_hop
                if (ie->ip.s_addr == next_hop->s_addr) {
                    helper = *next_hop;
                    helper.s_addr = htonl(helper.s_addr);
                    sprintf(ip_buffer2, "%s", inet_ntoa(helper));
                    // Ex: r1 is forwarding packet for 1.2.3.4 to next
                    // hop 1.2.3.5 over eth1 interface
                    printf("%s is forwarding packet for %s to next hop %s over"
                                   " %s interface.\n", r->name, ip_buffer1,
                           ip_buffer2, ie->interface_name);
                    rs_send_packet(p, top, i);
                    return;
                }
            }
        }
    }
    printf("%s is discarding packet %s because no destiny were found.\n",
           r->name, ip_buffer1);
}

void rs_debug(network_topology *data) {
    int i, j;
    struct in_addr helper; // Dummy var to show correct byte order to user
    router *r;
    routers_table_entry *re;
    interfaces_table_entry *ie;

    puts(DIV_LINE);
    puts("Routing's Table - Info");
    puts(MINOR_DIV_LINE);
    printf("Routers (n = %zu):\n", data->size);
    puts(MINOR_DIV_LINE);
    for (i = 0; i < data->size; i++) {
        r = &data->routers[i];
        printf("Name: %s\n", r->name);
        printf("Table values (size = %zu):\n", r->rt.size);
        for (j = 0; j < r->rt.size; j++) {
            re = &r->rt.table[j];
            printf("-> ");
            printf("IP: ");
            helper = re->ip;
            helper.s_addr = htonl(helper.s_addr);
            printf(inet_ntoa(helper));
            printf("; ");
            printf("Subnet mask (/%d): ", re->subnet_mask_prefix);
            helper = re->subnet_mask;
            helper.s_addr = htonl(helper.s_addr);
            printf(inet_ntoa(helper));
            printf("; ");
            printf("Gateway (net hop): ");
            helper = re->gateway;
            helper.s_addr = htonl(helper.s_addr);
            printf(inet_ntoa(helper));
            printf("; ");
            printf("Interface: %s\n", re->interface_name);
        }
        puts(MINOR_DIV_LINE);
    }
    puts(DIV_LINE);
    puts("Interface's Table - Info");
    puts(MINOR_DIV_LINE);
    printf("Interfaces (n = %zu):\n", data->size);
    puts(MINOR_DIV_LINE);
    for (i = 0; i < data->size; i++) {
        r = &data->routers[i];
        printf("Name: %s\n", r->name);
        printf("Table values (size = %zu):\n", r->it.size);
        for (j = 0; j < r->it.size; j++) {
            ie = &r->it.table[j];
            printf("-> ");
            printf("Interface: %s; ", ie->interface_name);
            printf("Subnet mask (/%d): ", ie->subnet_mask_prefix);
            helper = ie->subnet_mask;
            helper.s_addr = htonl(helper.s_addr);
            printf(inet_ntoa(helper));
            printf("; ");
            printf("IP: ");
            helper = ie->ip;
            helper.s_addr = htonl(helper.s_addr);
            printf(inet_ntoa(helper));
            printf("\n");
        }
        puts(MINOR_DIV_LINE);
    }
    puts(DIV_LINE);
}

void rs_close(network_topology *data) {
    if (!data)
        return;
    int i;
    for (i = 0; i < data->size; i++) {
        free(data->routers[i].rt.table);
        free(data->routers[i].it.table);
    }
    memset(data, 0, sizeof(network_topology));
}
