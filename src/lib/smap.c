#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/smap.h"

smap_t *smap_init(int size) {
    smap_t *smap = (smap_t *)malloc(sizeof(smap_t));

    smap->size = size;
    smap->slots = (smap_entry_t **)calloc(sizeof(smap_entry_t *), size);

    return smap;
}

void smap_entry_free(smap_entry_t *entry) {
    if (!entry)
        return;

    smap_entry_free(entry->next);
    mem_free(entry->key);
    mem_free(entry);
}

void smap_free(smap_t *t) {
    for (int i = 0; i < t->size; i++)
        smap_entry_free(t->slots[i]);
    mem_free(t->slots);
    mem_free(t);
}

unsigned int smap_hash_fun(char *key) {
    /* This is the djb2 algorithm as found for hashing strings.
     * Source: http://www.cse.yorku.ca/~oz/hash.html
     */

    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

unsigned int smap_hash(smap_t *t, char *key) {
    return smap_hash_fun(key) % t->size;
}

smap_entry_t *smap_entry_init(char *key, void *value) {
    smap_entry_t *entry = (smap_entry_t *)malloc(sizeof(smap_entry_t));

    entry->key = strdup(key);
    entry->value = value;
    entry->next = NULL;

    return entry;
}

void smap_insert(smap_t *t, char *key, void *value) {
    smap_entry_t *last;

    if (t->slots[smap_hash(t, key)]) {
        for (last = t->slots[smap_hash(t, key)]; last->next;
             last = last->next) {
            if (strcmp(last->key, key) == 0) {
                last->value = value;
                return;
            }
        }

        last->next = smap_entry_init(key, value);
    } else
        t->slots[smap_hash(t, key)] = smap_entry_init(key, value);
}

void *smap_retrieve(smap_t *t, char *key) {
    if (t->slots[smap_hash(t, key)])
        for (smap_entry_t *last = t->slots[smap_hash(t, key)]; last;
             last = last->next)
            if (strcmp(last->key, key) == 0)
                return last->value;

    return NULL;
}

void smap_map(smap_t *t, void *(f)(char *, void *)) {
    for (int i = 0; i < t->size; i++)
        for (smap_entry_t *last = t->slots[i]; last; last = last->next) {
            last->value = f(last->key, last->value);
        }
}
