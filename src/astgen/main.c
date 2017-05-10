#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "astgen/ast.h"
#include "astgen/check-ast.h"
#include "astgen/create-ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/free-ast.h"
#include "astgen/hash-ast.h"
#include "astgen/print-ast.h"
#include "astgen/sort-ast.h"

#include "astgen/gen-ast-definition.h"
#include "astgen/gen-binary-serialization.h"
#include "astgen/gen-consistency-functions.h"
#include "astgen/gen-copy-functions.h"
#include "astgen/gen-create-functions.h"
#include "astgen/gen-dot-definition.h"
#include "astgen/gen-free-functions.h"
#include "astgen/gen-pass-header.h"
#include "astgen/gen-phase-driver.h"
#include "astgen/gen-trav-functions.h"
#include "astgen/gen-user-trav-header.h"

extern struct Config *parse(FILE *fp);

char *yy_filename;

static void usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin)
        program = program_bin + 1;

    printf("Usage: %s [options] [file]\n", program);
    printf("Options:\n");
    printf("  --output-dir/-o <directory>  Directory to write generated "
           "files to.\n");
    printf("                               Defaults to ./src/generated/\n");
    printf("  --verbose/-v                 Enable verbose mode.\n");
    printf("  --dot <directory>            Will produce ast.dot in "
           "<directory>.\n");
    printf("                               Prints the AST after parsing the "
           "input file\n");
}

static void version(void) {
    printf("CoCoNut AST-gen 0.1\n");
}

static FILE *open_input_file(char *path) {

    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        fprintf(stderr, "%s: cannot open file: %s\n", path, strerror(errno));
        exit(1);
    }

    // Test if file a regular file.
    if (S_ISREG(path_stat.st_mode) != 1) {
        fprintf(stderr, "%s: cannot open file: %s\n", path,
                "file is not a regular file.");
        exit(1);
    }

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: cannot open file: %s\n", yy_filename,
                strerror(errno));
        exit(1);
    }

    return f;
}

int main(int argc, char *argv[]) {
    int verbose_flag = 0;
    int ret = 0;
    int option_index;
    int c = 0;
    char *header_dir = NULL;
    char *source_dir = NULL;
    char *dot_dir = NULL;

    struct option long_options[] = {{"verbose", no_argument, &verbose_flag, 1},
                                    {"header-dir", required_argument, 0, 21},
                                    {"source-dir", required_argument, 0, 22},
                                    {"dot", required_argument, 0, 23},
                                    {"help", no_argument, 0, 'h'},
                                    {"version", no_argument, 0, 0},
                                    {0, 0, 0, 0}};

    while (1) {
        c = getopt_long(argc, argv, "v", long_options, &option_index);

        // End of options
        if (c == -1)
            break;

        switch (c) {
        case 0:
            version();
            return 0;
        case 'v':
            verbose_flag = 1;
            break;
        case 21: // Header file output directory.
            header_dir = optarg;
            break;
        case 22: // Source file output directory.
            source_dir = optarg;
            break;
        case 23: // ast.dot output directory.
            dot_dir = optarg;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        case '?':
            usage(argv[0]);
            return 1;
        }
    }

    if (optind == argc - 1) {
        yy_filename = argv[optind];
    } else {
        usage(argv[0]);
        return 1;
    }

    if (header_dir == NULL)
        header_dir = "include/generated/";
    if (source_dir == NULL)
        source_dir = "src/generated/";

    FILE *f = open_input_file(yy_filename);

    struct Config *parse_result = parse(f);

    fclose(f);

    if (check_config(parse_result)) {
        fprintf(stderr, "\n\nFound errors\n");
        exit(1);
    }

    // Sort to prevent changes in order of attributes trigger regeneration of
    // code.
    sort_config(parse_result);
    hash_config(parse_result);

    if (verbose_flag) {
        print_config(parse_result);
    }

    if (dot_dir) {
        filegen_init(dot_dir);
        filegen_add("ast.dot", generate_dot_definition);
        ret += filegen_generate(parse_result);
        exit(ret);
    }

    // Generated all the header files.
    filegen_init(header_dir);
    filegen_add("enum.h", generate_enum_definitions);
    filegen_add("ast.h", generate_ast_definitions);

    // Free nodes.
    filegen_add("free-ast.h", generate_free_header);
    filegen_all_nodes("free-%s.h", generate_free_node_header);
    filegen_all_nodesets("free-%s.h", generate_free_nodeset_header);

    filegen_add("create-ast.h", generate_create_header);
    filegen_all_nodes("create-%s.h", generate_create_node_header);
    filegen_all_nodesets("create-%s.h", generate_create_nodeset_header);

    filegen_add("copy-ast.h", generate_copy_header);
    filegen_all_nodes("copy-%s.h", generate_copy_node_header);
    filegen_all_nodesets("copy-%s.h", generate_copy_nodeset_header);

    filegen_add("trav-ast.h", generate_trav_header);
    // filegen_add("consistency-ast.h", generate_consistency_header);
    filegen_add("phase-driver.h", generate_phase_driver_header);

    filegen_all_traversals("traversal-%s.h", generate_user_trav_header);
    filegen_all_passes("pass-%s.h", generate_pass_header);

    ret += filegen_generate(parse_result);

    // Genereate all the source files.
    filegen_init(source_dir);

    filegen_all_nodes("free-%s.c", generate_free_node_definitions);
    filegen_all_nodesets("free-%s.c", generate_free_nodeset_definitions);

    filegen_all_nodes("create-%s.c", generate_create_node_definitions);
    filegen_all_nodesets("create-%s.c", generate_create_nodeset_definitions);

    filegen_all_nodes("copy-%s.c", generate_copy_node_definitions);
    filegen_all_nodesets("copy-%s.c", generate_copy_nodeset_definitions);

    filegen_add("trav-ast.c", generate_trav_definitions);
    // filegen_add("consistency-ast.c", generate_consistency_definitions);
    filegen_add("phase-driver.c", generate_phase_driver_definitions);
    filegen_add("binary-serialization-write.c",
                generate_binary_serialization_definitions);

    ret += filegen_generate(parse_result);

    free_config(parse_result);

    return ret;
}
