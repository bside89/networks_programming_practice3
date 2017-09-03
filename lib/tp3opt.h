#ifndef TP3_TP3OPT_H
#define TP3_TP3OPT_H

#include <unistd.h>
#include <stdint.h>
#include <limits.h>

#define OPT_NOT_VALID_OPTION -1

/* Structure containing infos about options chosen by user at startup */
typedef struct {
    char        router_tables_file[PATH_MAX];
    char        interface_info_files[PATH_MAX];
    short int   debug_opt;
} rs_opt;

int rsopt_set(int, char **, rs_opt *);

#endif //TP3_TP3OPT_H
