#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "astgen/ast.h"

#include "lib/array.h"
#include "lib/memory.h"

#define COLOR_GREEN "\033[1m\033[32m"
#define COLOR_RESET "\033[0m"

enum FileGenType {
    FGT_basic,
    FGT_userdata,
    FGT_nodes,
    FGT_nodesets,
    FGT_traversal,
    FGT_pass,
};

struct FileGen {
    char *filename;
    union {
        void (*func)(struct Config *, FILE *);
        void (*func_userdata)(struct Config *, FILE *, void *);
        void (*func_node)(struct Config *, FILE *, struct Node *);
        void (*func_nodeset)(struct Config *, FILE *, struct Nodeset *);
        void (*func_traversal)(struct Config *, FILE *, struct Traversal *);
        void (*func_pass)(struct Config *, FILE *, struct Pass *);
    } function;
    void *main_func;
    void *user_data;
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

static FILE *get_fp(char *full_path) {
    FILE *fp = fopen(full_path, "w");
    if (!fp) {
        perror("Opening file failed");
        filegen_cleanup();
        exit(-1);
    }
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

    printf(COLOR_GREEN " GEN       " COLOR_RESET "%s\n", full_path);
    return full_path;
}

void filegen_add_with_userdata(char *filename,
                               void (*main_func)(struct Config *, FILE *,
                                                 void *),
                               void *user_data) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(filename);
    g->function.func_userdata = main_func;
    g->user_data = user_data;
    g->gen_type = FGT_userdata;
    array_append(file_generations, g);
}

void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *)) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(filename);
    g->function.func = main_func;
    g->user_data = NULL;
    g->gen_type = FGT_basic;
    array_append(file_generations, g);
}

void filegen_all_nodes(char *fileformatter,
                       void (*main_func)(struct Config *, FILE *,
                                         struct Node *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_node = main_func;
    g->user_data = NULL;
    g->gen_type = FGT_nodes;
    array_append(file_generations, g);
}

void filegen_all_nodesets(char *fileformatter,
                          void (*main_func)(struct Config *, FILE *,
                                            struct Nodeset *)) {
    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->filename = strdup(fileformatter);
    g->function.func_nodeset = main_func;
    g->user_data = NULL;
    g->gen_type = FGT_nodesets;
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
            fp = get_fp(full_path);

            g->function.func(config, fp);

            fclose(fp);
            mem_free(full_path);
            break;

        case FGT_userdata:
            full_path = get_full_path(g->filename, NULL, out_dir_len);
            fp = get_fp(full_path);

            g->function.func_userdata(config, fp, g->user_data);

            fclose(fp);
            mem_free(full_path);
            break;

        case FGT_nodes:
            for (int i = 0; i < array_size(config->nodes); ++i) {
                struct Node *node = array_get(config->nodes, i);
                full_path = get_full_path(g->filename, node->id, out_dir_len);
                fp = get_fp(full_path);

                g->function.func_node(config, fp, node);

                fclose(fp);
                mem_free(full_path);
            }
            break;
        case FGT_nodesets:
            for (int i = 0; i < array_size(config->nodesets); ++i) {
                struct Nodeset *nodeset = array_get(config->nodesets, i);
                full_path =
                    get_full_path(g->filename, nodeset->id, out_dir_len);
                fp = get_fp(full_path);

                g->function.func_nodeset(config, fp, nodeset);

                fclose(fp);
                mem_free(full_path);
            }
            break;
            break;
        case FGT_traversal:
            break;
        case FGT_pass:
            break;
        }
    }

    filegen_cleanup();
    return 0;
}
