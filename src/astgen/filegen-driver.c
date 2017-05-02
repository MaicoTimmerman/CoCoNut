#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "astgen/ast.h"

#include "lib/array.h"
#include "lib/memory.h"

struct FileGen {
    char *fn;
    union {
        void (*func)(struct Config *, FILE *);
        void (*func_userdata)(struct Config *, FILE *, void *);
    } function;
    void *main_func;
    void *user_data;
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

void filegen_add_with_userdata(char *filename,
                               void (*main_func)(struct Config *, FILE *,
                                                 void *),
                               void *user_data) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->fn = strdup(filename);
    g->function.func_userdata = main_func;
    g->user_data = user_data;
    array_append(file_generations, g);
}

void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *)) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->fn = strdup(filename);
    g->function.func = main_func;
    g->user_data = NULL;
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
                return -1;
            } else {
                // Directory already exists
                closedir(d);
            }
        } else {
            perror("Creation failed");
            return -1;
        }
    }

    size_t out_dir_len = strlen(output_directory);

    for (int i = 0; i < array_size(file_generations); i++) {
        struct FileGen *g = array_get(file_generations, i);
        char *full_path = mem_alloc(out_dir_len + strlen(g->fn) + 1);
        strcpy(full_path, output_directory);
        strcat(full_path, g->fn);

        printf("Generating %s\n", full_path);

        FILE *fp = fopen(full_path, "w");
        if (!fp) {
            perror("Opening file failed");
            return -1;
        }

        if (g->user_data == NULL) {
            g->function.func(config, fp);

        } else {
            g->function.func_userdata(config, fp, g->user_data);
        }

        fclose(fp);

        mem_free(full_path);
    }

    return 0;
}

static void free_filegen(void *p) {
    struct FileGen *g = (struct FileGen *)p;
    mem_free(g->fn);
    mem_free(g);
}

void filegen_cleanup(void) {
    array_cleanup(file_generations, free_filegen);
    mem_free(output_directory);
}
