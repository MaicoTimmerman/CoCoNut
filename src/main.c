#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "check-ast.h"
#include "create-ast.h"
#include "filegen-driver.h"
#include "free-ast.h"
#include "print-ast.h"

#include "gen-ast-definition.h"
#include "gen-consistency-functions.h"
#include "gen-copy-functions.h"
#include "gen-create-functions.h"
#include "gen-free-functions.h"
#include "gen-trav-functions.h"
#include "print-ast.h"

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
    printf("                               Prints the AST after parsing the "
           "input file\n");
}

static void version(void) {
    printf("CoCoNut AST-gen 0.1\n");
}

int main(int argc, char *argv[]) {
    int verbose_flag;
    int option_index;
    int c = 0;
    char *output_dir = NULL;

    struct option long_options[] = {{"verbose", no_argument, &verbose_flag, 1},
                                    {"output-dir", required_argument, 0, 'o'},
                                    {"help", no_argument, 0, 'h'},
                                    {"version", no_argument, 0, 0},
                                    {0, 0, 0, 0}};

    while (1) {
        c = getopt_long(argc, argv, "vo:", long_options, &option_index);

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
        case 'o':
            output_dir = optarg;
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

    if (output_dir == NULL) {
        output_dir = "src/generated/";
    }

    FILE *f = fopen(yy_filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: cannot open file: %s\n", yy_filename,
                strerror(errno));
        return 1;
    }

    struct Config *parse_result = parse(f);

    fclose(f);

    if (check_config(parse_result)) {
        fprintf(stderr, "\n\nFound errors\n");
        exit(1);
    }

    if (verbose_flag) {
        print_config(parse_result);
    }

    filegen_init(output_dir);
    filegen_add("enum.h", generate_enum_definitions);
    filegen_add("ast.h", generate_ast_definitions);

    filegen_add("free-ast.h", generate_free_header);
    filegen_add("free-ast.c", generate_free_definitions);

    filegen_add("create-ast.c", generate_create_functions);
    filegen_add("create-ast.h", generate_create_function_headers);

    filegen_add("trav-ast.h", generate_trav_header);
    filegen_add("trav-ast.c", generate_trav_definitions);

    filegen_add("copy-ast.h", generate_copy_header);
    filegen_add("copy-ast.c", generate_copy_definitions);

    filegen_add("consistency-ast.h", generate_consistency_header);
    filegen_add("consistency-ast.c", generate_consistency_definition);

    int ret = filegen_generate(parse_result);
    filegen_cleanup();

    free_config(parse_result);

    return ret;
}
