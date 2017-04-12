#include <stdio.h>
#include <stdlib.h>

void *mem_alloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation returned NULL-ptr");
        exit(1);
    }
    return ptr;
}

void *mem_free(void *ptr) {
    if (ptr != NULL)
        free(ptr);
    return NULL;
}
