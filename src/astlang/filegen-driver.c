#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "array.h"
#include "ast.h"
#include "memory.h"

struct FileGen {
    char *fn;
    void (*main_func)(struct Config *config, FILE *fp);
};

static array *file_generations = NULL;
static char *output_directory = NULL;

void filegen_init(char *out_dir) {
    file_generations = array_init(4);
    output_directory = strdup(out_dir);
}

void filegen_add(char *filename,
                 void (*main_func)(struct Config *config, FILE *fp)) {

    struct FileGen *g = malloc(sizeof(struct FileGen));
    g->fn = strdup(filename);
    g->main_func = main_func;
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

        g->main_func(config, fp);

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
