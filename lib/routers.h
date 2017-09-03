#ifndef TP3_ROUTERS_H
#define TP3_ROUTERS_H

#include <netinet/in.h>
#include <ifaddrs.h>

#define INTERFACE_NAME_MAX_LENGTH 30

typedef struct {
    struct in_addr ip;
    struct in_addr subnet_mask;
    struct in_addr gateway;
    char interface_name[INTERFACE_NAME_MAX_LENGTH];
} router_table_entry;

typedef struct {
    size_t size;
    router_table_entry *table;
} router_table;

#endif //TP3_ROUTERS_H
