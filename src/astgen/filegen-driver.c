#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "astgen/ast.h"
#include "astgen/filegen-util.h"

#include "lib/array.h"
#include "lib/memory.h"

#define COLOR_GREEN "\033[1m\033[32m"
#define COLOR_RESET "\033[0m"
#define HASH_HEADER "// Hash: %s\n"

enum FileGenType {
    FGT_basic,
    FGT_nodes,
    FGT_nodesets,
    FGT_traversal,
    FGT_pass,
};

struct FileGen {
    char *filename;
    union {
        void (*func)(struct Config *, FILE *);
        void (*func_node)(struct Config *, FILE *, struct Node *);
        void (*func_nodeset)(struct Config *, FILE *, struct Nodeset *);
        void (*func_traversal)(struct Config *, FILE *, struct Traversal *);
        void (*func_pass)(struct Config *, FILE *, struct Pass *);
    } function;
    void *main_func;
    enum FileGenType gen_type;
};

static array *file_generations = NULL;
static char *output_directory = NULL;

void filegen_init(char *out_dir) {
    file_generations = array_init(4);

    size_t out_dir_len = strlen(out_dir);
    if (out_dir[out_dir_len - 1] != '/') {
        output_directory = mem_alloc(out_dir_len + 2);
        sprintf(output_directory, "%s/", out_dir);
    } else {
        output_directory = strdup(out_dir);
    }
}

static void free_filegen(void *p) {
    struct FileGen *g = (struct FileGen *)p;
    mem_free(g->filename);
    mem_free(g);
}

static void filegen_cleanup(void) {
    array_cleanup(file_generations, free_filegen);
    mem_free(output_directory);
}

static FILE *get_write_fp(char *full_path) {
    FILE *fp = fopen(full_path, "w");
    if (!fp) {
        perror("Opening file failed");
        filegen_cleanup();
        exit(-1);
    }
    return fp;
}

static FILE *get_read_fp(char *full_path) {
    FILE *fp = fopen(full_path, "r");
    return fp;
}

static char *get_full_path(char *filename, char *formatter,
                           size_t out_dir_len) {
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

    FILE *fp = get_read_fp(full_path);
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

void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *)) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(filename);
    g->function.func = main_func;
    g->gen_type = FGT_basic;
    array_append(file_generations, g);
}

void filegen_all_nodes(char *fileformatter,
                       void (*main_func)(struct Config *, FILE *,
                                         struct Node *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_node = main_func;
    g->gen_type = FGT_nodes;
    array_append(file_generations, g);
}

void filegen_all_nodesets(char *fileformatter,
                          void (*main_func)(struct Config *, FILE *,
                                            struct Nodeset *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_nodeset = main_func;
    g->gen_type = FGT_nodesets;
    array_append(file_generations, g);
}

void filegen_all_traversals(char *fileformatter,
                            void (*main_func)(struct Config *, FILE *,
                                              struct Traversal *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_traversal = main_func;
    g->gen_type = FGT_traversal;
    array_append(file_generations, g);
}

void filegen_all_passes(char *fileformatter,
                        void (*main_func)(struct Config *, FILE *,
                                          struct Pass *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_pass = main_func;
    g->gen_type = FGT_pass;
    array_append(file_generations, g);
}

int filegen_generate(struct Config *config) {

    if (mkdir(output_directory, 0755) == 0) {
        printf("Created output directory %s\n", output_directory);
    } else {
        if (errno == EEXIST) {
            DIR *d = opendir(output_directory);

            // out_dir exists but is a file
            if (!d) {
                perror("Creation failed");
                filegen_cleanup();
                return -1;
            } else {
                // Directory already exists
                closedir(d);
            }
        } else {
            perror("Creation failed");
            filegen_cleanup();
            return -1;
        }
    }

    size_t out_dir_len = strlen(output_directory);
    char *full_path;
    FILE *fp;

    for (int i = 0; i < array_size(file_generations); i++) {
        struct FileGen *g = array_get(file_generations, i);
        switch (g->gen_type) {
        case FGT_basic:
            full_path = get_full_path(g->filename, NULL, out_dir_len);
            fp = get_write_fp(full_path);
            g->function.func(config, fp);
            mem_free(full_path);
            break;

        case FGT_nodes:
            for (int i = 0; i < array_size(config->nodes); ++i) {
                struct Node *node = array_get(config->nodes, i);
                full_path = get_full_path(g->filename, node->id, out_dir_len);

                if (hash_match(node->common_info, full_path)) {
                    mem_free(full_path);
                    continue;
                }

                fp = get_write_fp(full_path);
                out(HASH_HEADER, node->common_info->hash);
                g->function.func_node(config, fp, node);
            }
            break;

        case FGT_nodesets:
            for (int i = 0; i < array_size(config->nodesets); ++i) {
                struct Nodeset *nodeset = array_get(config->nodesets, i);
                full_path =
                    get_full_path(g->filename, nodeset->id, out_dir_len);

                if (hash_match(nodeset->common_info, full_path)) {
                    mem_free(full_path);
                    continue;
                }

                fp = get_write_fp(full_path);
                out(HASH_HEADER, nodeset->common_info->hash);
                g->function.func_nodeset(config, fp, nodeset);
            }
            break;

        case FGT_traversal:
            for (int i = 0; i < array_size(config->traversals); ++i) {
                struct Traversal *traversal = array_get(config->traversals, i);
                full_path =
                    get_full_path(g->filename, traversal->id, out_dir_len);

                if (hash_match(traversal->common_info, full_path)) {
                    mem_free(full_path);
                    continue;
                }

                fp = get_write_fp(full_path);
                out(HASH_HEADER, traversal->common_info->hash);
                g->function.func_traversal(config, fp, traversal);
            }
            break;

        case FGT_pass:
            for (int i = 0; i < array_size(config->passes); ++i) {
                struct Pass *pass = array_get(config->passes, i);
                full_path = get_full_path(g->filename, pass->id, out_dir_len);

                if (hash_match(pass->common_info, full_path)) {
                    mem_free(full_path);
                    continue;
                }

                fp = get_write_fp(full_path);
                out(HASH_HEADER, pass->common_info->hash);
                g->function.func_pass(config, fp, pass);
            }
            break;
            fclose(fp);
            mem_free(full_path);
        }
    }

    filegen_cleanup();
    return 0;
}
