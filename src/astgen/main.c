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
#include "astgen/print-ast.h"

#include "astgen/gen-ast-definition.h"
#include "astgen/gen-consistency-functions.h"
#include "astgen/gen-copy-functions.h"
#include "astgen/gen-create-functions.h"
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
    int option_index;
    int c = 0;
    char *header_dir = NULL;
    char *source_dir = NULL;

    struct option long_options[] = {{"verbose", no_argument, &verbose_flag, 1},
                                    {"header-dir", required_argument, 0, 21},
                                    {"source-dir", required_argument, 0, 22},
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

    if (verbose_flag) {
        print_config(parse_result);
    }

    // Generated all the header files.
    filegen_init(header_dir);
    filegen_add("enum.h", generate_enum_definitions);
    filegen_add("ast.h", generate_ast_definitions);

    filegen_add("free-ast.h", generate_free_header);
    filegen_add("create-ast.h", generate_create_header);
    filegen_add("trav-ast.h", generate_trav_header);
    filegen_add("copy-ast.h", generate_copy_header);
    filegen_add("consistency-ast.h", generate_consistency_header);
    filegen_add("phase-driver.h", generate_phase_driver_header);

    for (int i = 0; i < array_size(parse_result->traversals); i++) {
        struct Traversal *trav = array_get(parse_result->traversals, i);
        char header[13 + strlen(trav->id)];
        sprintf(header, "traversal-%s.h", trav->id);
        filegen_add_with_userdata(header, generate_user_trav_header, trav);
    }

    for (int i = 0; i < array_size(parse_result->passes); i++) {
        struct Pass *pass = array_get(parse_result->passes, i);
        char header[8 + strlen(pass->id)];
        sprintf(header, "pass-%s.h", pass->id);
        filegen_add_with_userdata(header, generate_pass_header, pass);
    }

    int ret;
    ret = filegen_generate(parse_result);
    filegen_cleanup();

    // Genereate all the source files.
    filegen_init(source_dir);
    filegen_add("free-ast.c", generate_free_definitions);
    filegen_add("create-ast.c", generate_create_definitions);
    filegen_add("trav-ast.c", generate_trav_definitions);
    filegen_add("copy-ast.c", generate_copy_definitions);
    filegen_add("consistency-ast.c", generate_consistency_definitions);
    filegen_add("phase-driver.c", generate_phase_driver_definitions);

    ret = filegen_generate(parse_result);
    filegen_cleanup();

    free_config(parse_result);

    return ret;
}
