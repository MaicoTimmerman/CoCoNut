#pragma once

#define HTABLE_SIZE 20

typedef struct htable_entry_t {
    char *key;
    void *value;
    struct htable_entry_t *next;
} htable_entry_t;

typedef struct htable_t {
    int size;
    struct htable_entry_t **slots;
} htable_t;

htable_t *htable_init(int);
void htable_free(htable_t *);
unsigned int htable_hash_fun(char *);
unsigned int htable_hash(htable_t *, char *);
htable_entry_t *htable_entry_init(char *, void *);
void htable_insert(htable_t *, char *, void *);
void *htable_retrieve(htable_t *, char *);
void htable_map(htable_t *, void *(f)(char *, void *));
