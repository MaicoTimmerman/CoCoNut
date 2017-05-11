#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "astgen/ast.h"
#include "astgen/errors.h"
#include "astgen/filegen-util.h"

#include "lib/array.h"
#include "lib/memory.h"

#define COLOR_GREEN "\033[1m\033[32m"
#define COLOR_RESET "\033[0m"
#define HASH_HEADER "// Hash: %s\n"

static Config *ast_definition = NULL;
static char *output_directory = NULL;

static FILE *get_fp(char *full_path) {
    FILE *fp = fopen(full_path, "w");
    if (!fp) {
        perror("Opening file failed");
        exit(CANNOT_OPEN_FILE);
    }
    return fp;
}

static char *get_full_path(char *filename, char *formatter) {
    size_t out_dir_len = strlen(output_directory);
    char *full_path = mem_alloc(out_dir_len + strlen(filename) + 1);
    strcpy(full_path, output_directory);
    strcat(full_path, filename);

    if (formatter) {
        char *old_full_path = full_path;
        full_path =
            mem_alloc(out_dir_len + strlen(formatter) + strlen(filename) + 1);
        sprintf(full_path, old_full_path, formatter);
        mem_free(old_full_path);
    }

    return full_path;
}

static bool hash_match(NodeCommonInfo *info, char *full_path) {

    char *current_hash = mem_alloc(43 * sizeof(char));
    bool rv = false;

    FILE *fp = fopen(full_path, "r");
    if (fp == NULL)
        return rv;

    // Hash: %32s\n -> read 3+4+2+32+1 = 42 characters.
    if (fgets(current_hash, 42, fp) != NULL) {
        if (strncmp(current_hash + 9, info->hash, 32) == 0) {
            rv = true;
            printf(COLOR_GREEN " SAME      " COLOR_RESET "%s\n", full_path);
            goto cleanup;
        } else {
            printf(COLOR_GREEN " GEN       " COLOR_RESET "%s\n", full_path);
            goto cleanup;
        }
    }

cleanup:
    mem_free(current_hash);
    fclose(fp);
    return rv;
}

void filegen_init(Config *config) {
    ast_definition = config;
}

void filegen_dir(char *out_dir) {
    size_t out_dir_len = strlen(out_dir);

    if (out_dir[out_dir_len - 1] != '/') {
        output_directory = mem_alloc(out_dir_len + 2);
        sprintf(output_directory, "%s/", out_dir);
    } else {
        output_directory = strdup(out_dir);
    }

    if (mkdir(output_directory, 0755) == 0) {
        printf("Created output directory %s\n", output_directory);
    } else {
        if (errno == EEXIST) {
            DIR *d = opendir(output_directory);

            // out_dir exists but is a file
            if (!d) {
                perror("Creation failed");
                exit(CANNOT_CREATE_DIR);
            } else {
                // Directory already exists
                closedir(d);
            }
        } else {
            perror("Creation failed");
            exit(CANNOT_CREATE_DIR);
        }
    }
}

void filegen_generate(char *filename, void (*func)(Config *, FILE *)) {

    char *full_path = get_full_path(filename, NULL);
    FILE *fp = get_fp(full_path);

    func(ast_definition, fp);

    mem_free(full_path);
    fclose(fp);
}

void filegen_all_nodes(char *fileformatter,
                       void (*func)(Config *, FILE *, Node *)) {
    char *full_path;
    FILE *fp;

    for (int i = 0; i < array_size(ast_definition->nodes); ++i) {
        Node *node = array_get(ast_definition->nodes, i);
        full_path = get_full_path(fileformatter, node->id);

        if (hash_match(node->common_info, full_path)) {
            mem_free(full_path);
            continue;
        }

        fp = get_fp(full_path);
        out(HASH_HEADER, node->common_info->hash);
        func(ast_definition, fp, node);
        mem_free(full_path);
        fclose(fp);
    }
}

void filegen_all_nodesets(char *fileformatter,
                          void (*func)(Config *, FILE *, Nodeset *)) {
    char *full_path;
    FILE *fp;

    for (int i = 0; i < array_size(ast_definition->nodesets); ++i) {
        Nodeset *nodeset = array_get(ast_definition->nodesets, i);
        full_path = get_full_path(fileformatter, nodeset->id);

        if (hash_match(nodeset->common_info, full_path)) {
            mem_free(full_path);
            continue;
        }

        fp = get_fp(full_path);
        out(HASH_HEADER, nodeset->common_info->hash);
        func(ast_definition, fp, nodeset);
        mem_free(full_path);
        fclose(fp);
    }
}

void filegen_all_traversals(char *fileformatter,
                            void (*func)(Config *, FILE *, Traversal *)) {
    char *full_path;
    FILE *fp;

    for (int i = 0; i < array_size(ast_definition->traversals); ++i) {
        Traversal *traversal = array_get(ast_definition->traversals, i);
        full_path = get_full_path(fileformatter, traversal->id);

        if (hash_match(traversal->common_info, full_path)) {
            mem_free(full_path);
            continue;
        }

        fp = get_fp(full_path);
        out(HASH_HEADER, traversal->common_info->hash);
        func(ast_definition, fp, traversal);
        mem_free(full_path);
        fclose(fp);
    }
}

void filegen_all_passes(char *fileformatter,
                        void (*func)(Config *, FILE *, Pass *)) {
    char *full_path;
    FILE *fp;

    for (int i = 0; i < array_size(ast_definition->passes); ++i) {
        Pass *pass = array_get(ast_definition->passes, i);
        full_path = get_full_path(fileformatter, pass->id);

        if (hash_match(pass->common_info, full_path)) {
            mem_free(full_path);
            continue;
        }

        fp = get_fp(full_path);
        out(HASH_HEADER, pass->common_info->hash);
        func(ast_definition, fp, pass);
        mem_free(full_path);
        fclose(fp);
    }
}
