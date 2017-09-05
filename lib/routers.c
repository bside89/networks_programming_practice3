#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "routers.h"
#include "common.h"
#include "packet.h"

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
        memset(ip_buffer, 0, sizeof(ip_buffer));
        sscanf(buffer_line, "%s", ip_buffer);
        inet_aton(ip_buffer, &r->ip); // Define IP

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

            // Define subnet mask  (registry)
            pkt_generate_netmask(re->subnet_mask_prefix, &re->subnet_mask);

            // Define gateway IP (next hop) (registry)
            inet_aton(gw_buffer, &re->gateway);
        }
    }
}

void rs_get_interface_tables_data(FILE *source, network_topology *data) {
    int i, j;
    router *r;
    interfaces_table_entry *ie;
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

            // Define subnet mask  (registry)
            pkt_generate_netmask(ie->subnet_mask_prefix, &ie->subnet_mask);

            // Define IP (registry)
            inet_aton(ip_buffer, &ie->ip);
        }
    }
}

void rs_debug(network_topology *data) {
    int i, j;
    struct in_addr tmp;
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
        printf("IP: ");
        printf(inet_ntoa(r->ip));
        putchar('\n');
        printf("Table values (size = %zu):\n", r->rt.size);
        for (j = 0; j < r->rt.size; j++) {
            re = &r->rt.table[j];
            printf("-> ");
            printf("IP: ");
            printf(inet_ntoa(re->ip));
            printf("; ");
            printf("Subnet mask (/%d): ", re->subnet_mask_prefix);
            tmp = re->subnet_mask;
            tmp.s_addr = htonl(tmp.s_addr);
            printf(inet_ntoa(tmp));
            printf("; ");
            printf("Gateway (net hop): ");
            printf(inet_ntoa(re->gateway));
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
            tmp = ie->subnet_mask;
            tmp.s_addr = htonl(tmp.s_addr);
            printf(inet_ntoa(tmp));
            printf("; ");
            printf("IP: ");
            printf(inet_ntoa(ie->ip));
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
        data->routers[i].rt.table = NULL;
        data->routers[i].it.table = NULL;
    }
}
