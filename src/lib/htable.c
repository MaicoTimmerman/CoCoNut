#include "htable.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

htable_t *htable_init(int size) {
    htable_t *htable = (htable_t *)malloc(sizeof(htable_t));

    htable->size = size;
    htable->slots = (htable_entry_t **)calloc(sizeof(htable_entry_t *), size);

    return htable;
}

void htable_entry_free(htable_entry_t *entry) {
    if (!entry)
        return;

    htable_entry_free(entry->next);
    free(entry->key);
    free(entry);
}

void htable_free(htable_t *t) {
    for (int i = 0; i < t->size; i++)
        htable_entry_free(t->slots[i]);
    free(t->slots);
    free(t);
}

unsigned int htable_hash_fun(char *key) {
    /* This is the djb2 algorithm as found for hashing strings.
     * Source: http://www.cse.yorku.ca/~oz/hash.html
     */

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

unsigned int htable_hash(htable_t *t, char *key) {
    return htable_hash_fun(key) % t->size;
}

htable_entry_t *htable_entry_init(char *key, void *value) {
    htable_entry_t *entry = (htable_entry_t *)malloc(sizeof(htable_entry_t));

    entry->key = strdup(key);
    entry->value = value;
    entry->next = NULL;

    return entry;
}

void htable_insert(htable_t *t, char *key, void *value) {
    htable_entry_t *last;

    if (t->slots[htable_hash(t, key)]) {
        for (last = t->slots[htable_hash(t, key)]; last->next;
             last = last->next) {
            if (strcmp(last->key, key) == 0) {
                last->value = value;
                return;
            }
        }

        last->next = htable_entry_init(key, value);
    } else
        t->slots[htable_hash(t, key)] = htable_entry_init(key, value);
}

void *htable_retrieve(htable_t *t, char *key) {
    if (t->slots[htable_hash(t, key)])
        for (htable_entry_t *last = t->slots[htable_hash(t, key)]; last;
             last = last->next)
            if (strcmp(last->key, key) == 0)
                return last->value;

    return NULL;
}

void htable_map(htable_t *t, void *(f)(char *, void *)) {
    for (int i = 0; i < t->size; i++)
        for (htable_entry_t *last = t->slots[i]; last; last = last->next)
            f(last->key, last->value);
}
