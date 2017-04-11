#ifndef ARRAY_H
#define ARRAY_H

struct array;

typedef struct array array;

/* Return a pointer to an empty dynamic array with 'initial capacity'
 * of storage allocated.
 * Return NULL if an error occured. */
struct array *array_init(long initial_capacity);

/* Free all elements stored in the array 'a' using the 'free_func()'
 * parameter, then free the array itself. If 'free_func' is NULL the
 * standard free() is used. */
void array_cleanup(struct array *a, void free_func(void *));

/* Set element at 'index' to 'p'.
 * Return 0 if succesful, -1 otherwise. */
int array_set(struct array *a, int index, void *p);

/* Return element at 'index' or NULL if an error occured. */
void *array_get(struct array *a, int index);

/* Append 'p' to the end of array 'a'.
 * Return 0 if succesful, -1 otherwise. */
int array_append(struct array *a, void *p);

/* Remove the last element of array 'a' and return it.
 * Return NULL if 'a' is empty. */
void *array_pop(struct array *a);

/* Return the size of array 'a'. */
int array_size(struct array *a);

/* Remove all elements from array 'a' */
void array_clear(struct array *a);

#endif
