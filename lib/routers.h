#ifndef TP3_ROUTERS_H
#define TP3_ROUTERS_H

#include <netinet/in.h>
#include <ifaddrs.h>

#define ROUTER_NAME_MAX_LENGTH 8
#define INTERFACE_NAME_MAX_LENGTH 20

typedef struct {
    struct in_addr ip;
    struct in_addr subnet_mask;
    struct in_addr gateway;
    char interface_name[INTERFACE_NAME_MAX_LENGTH];
} routers_table_entry;

typedef struct {
    size_t size;
    routers_table_entry *table;
} routers_table;

typedef struct {
    char name[ROUTER_NAME_MAX_LENGTH];
    routers_table table;
} router;

#endif //TP3_ROUTERS_H
