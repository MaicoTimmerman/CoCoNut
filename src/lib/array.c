#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/array.h"
#include "lib/memory.h"

struct array {
    void **data;
    long size;
    long capacity;
};

array *create_array(void) {
    return array_init(32);
}

struct array *array_init(long initial_capacity) {
    assert(initial_capacity > 0);

    struct array *a = mem_alloc(sizeof(struct array));
    a->data = (void **)mem_alloc(initial_capacity * sizeof(void *));
    a->size = 0;
    a->capacity = initial_capacity;
    return a;
}

static void noop(void *p) {}

void array_cleanup(struct array *a, void free_func(void *)) {
    void *e;
    if (free_func == NULL) {
        free_func = noop;
    }
    while ((e = array_pop(a))) {
        free_func(e);
    }
    free(a->data);
    free(a);
}

int array_set(struct array *a, int index, void *p) {
    if (index >= a->size)
        return -1;

    a->data[index] = p;
    return 0;
}

void *array_get(struct array *a, int index) {
    if (index < a->size)
        return a->data[index];
    else
        return NULL;
}

int array_append(struct array *a, void *p) {
    if (a->size == a->capacity) {
        a->capacity *= 2;
        a->data = realloc(a->data, a->capacity * sizeof(void *));
    }
    a->size++;
    return array_set(a, a->size - 1, p);
}

void *array_pop(struct array *a) {
    if (a->size == 0)
        return NULL;

    void *last = a->data[a->size - 1];
    a->size--;
    return last;
}

int array_size(struct array *a) {
    if (a == NULL)
        return 0;
    return a->size;
}

void array_clear(struct array *a) {
    a->size = 0;
}

void array_sort(struct array *a, int (*compare)(const void *, const void *)) {
    qsort(a->data, a->size, sizeof(void *), compare);
}
