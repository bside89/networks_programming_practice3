#ifndef TP3_ROUTERS_H
#define TP3_ROUTERS_H

#include <netinet/in.h>
#include <ifaddrs.h>
#include <stdio.h>

#define ROUTER_NAME_MAX_LENGTH 8
#define INTERFACE_NAME_MAX_LENGTH 20
#define MAX_ROUTERS 50
#define IP_ADDRESS_SIZE 16

// Structure containing entries for routing's table
typedef struct {
    struct in_addr ip;
    int subnet_mask_prefix;
    struct in_addr subnet_mask;
    struct in_addr gateway;
    char interface_name[INTERFACE_NAME_MAX_LENGTH];
} routers_table_entry;

// Routing's table
typedef struct {
    size_t size;
    routers_table_entry *table; // Need memory allocation
} routers_table;

// Entry for interface's table
typedef struct {
    char interface_name[INTERFACE_NAME_MAX_LENGTH];
    struct in_addr ip;
    int subnet_mask_prefix;
    struct in_addr subnet_mask;
} interfaces_table_entry;

// Interface's table
typedef struct {
    size_t size;
    interfaces_table_entry *table;  // Need memory allocation
} interfaces_table;

// Router
typedef struct {
    char name[ROUTER_NAME_MAX_LENGTH];
    struct in_addr ip;
    routers_table rt;
    interfaces_table it;
} router;

// All data
typedef struct {
    size_t size;
    router routers[MAX_ROUTERS];
} network_topology;

int rs_open(const char *, const char *, network_topology *);

void rs_get_routing_tables_data(FILE *source, network_topology *data);

void rs_get_interface_tables_data(FILE *source, network_topology *data);

void rs_debug(network_topology *);

void rs_close(network_topology *);

#endif //TP3_ROUTERS_H
