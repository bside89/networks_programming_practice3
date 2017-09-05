#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tp3opt.h"
#include "common.h"

// Options (getopt)

#define GETOPT_OPTIONS  "r:f:d"
#define NUM_OPTS        3

typedef enum { R_FLAG = 0, F_FLAG, DB_FLAG } opts_index;

#define OPT_ROUTER_TABLES_FILE  'r'
#define OPT_INTERF_INFO_FILE    'f'
#define OPT_DEBUG               'd'

const int opts_ch[] = {OPT_ROUTER_TABLES_FILE, OPT_INTERF_INFO_FILE, OPT_DEBUG};

void rsopt_debug(rs_opt *o);

int rsopt_is_option_valid(int mode);

int rsopt_set(int argc, char **argv, rs_opt *o) {
    int global_error = 0;
    int c;
    int i;
    int flags[NUM_OPTS];
    memset(o, 0, sizeof(*o));
    memset(flags, 0, NUM_OPTS * sizeof(int));
    opterr = 0;
    while ((c = getopt(argc, argv, GETOPT_OPTIONS)) != -1) {
        switch (c) {
            case OPT_ROUTER_TABLES_FILE:
                if (!rsopt_is_option_valid(flags[R_FLAG]))
                    return OPT_NOT_VALID_OPTION;
                strcpy(o->router_tables_file, optarg);
                flags[R_FLAG] = 1;
                break;
            case OPT_INTERF_INFO_FILE:
                if (!rsopt_is_option_valid(flags[F_FLAG]))
                    return OPT_NOT_VALID_OPTION;
                strcpy(o->interface_info_files, optarg);
                flags[F_FLAG] = 1;
                break;
            case OPT_DEBUG:
                if (!rsopt_is_option_valid(flags[DB_FLAG]))
                    return OPT_NOT_VALID_OPTION;
                o->debug_opt = 1;
                flags[DB_FLAG] = 1;
                break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return OPT_NOT_VALID_OPTION;
            default:
                exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < NUM_OPTS - 1; i++) {
        if (!flags[i]) {
            fprintf(stderr, "Option -%c is required.\n", opts_ch[i]);
            global_error = 1;
        }
    }
    for (i = optind; i < argc; i++) {
        // Has optional <args>
        fprintf(stderr, "Invalid option: %s\n", argv[i]);
        global_error = 1;
    }
    if (global_error)
        return OPT_NOT_VALID_OPTION;
    if (o->debug_opt)
        rsopt_debug(o);
    return 0;
}

int rsopt_is_option_valid(int mode) {
    if (mode) {
        fprintf(stderr, "Invalid options combinations.\n");
        return 0;
    }
    return 1;
}

void rsopt_debug(rs_opt *o) {
    puts(DIV_LINE);
    puts("ROUTER SIMULATOR OPT INFO (DEBUG MODE)");
    puts("File config:");
    printf("-> Router's table:    <%s>\n", o->router_tables_file);
    printf("-> Interface's table: <%s>\n", o->interface_info_files);
    puts(DIV_LINE);
}
