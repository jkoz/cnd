#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "hashtable.h"

static int _h1(const hashtable *ht, const void* key) {
    return *(const char*) key % ht->capacity;
}

static int _h2(const hashtable *ht, const void* key) {
    return 1 + *(const char*) key % (ht->capacity - 1);
}

hashtable* hashtable_create(int capacity, void (*drop)(void *item))
{
    hashtable *ht = malloc(sizeof(*ht));
    if ((ht->buckets = calloc(capacity, sizeof(entry*))) == NULL) {
        free(ht);
        return NULL;
    }
    ht->capacity = capacity;
    ht->drop = drop;
    ht->size = 0;
    return ht;
}

void hashtable_drop(hashtable *ht)
{
    if (ht->drop != NULL) {
        for (int i = 0; i < ht->capacity; i++) {
        if (ht->buckets[i] != NULL) { 
            if (ht->buckets[i]->key != NULL) {
                free(ht->buckets[i]->key);
                if (ht->buckets[i]->drop != NULL) {
                    ht->buckets[i]->drop(ht->buckets[i]->data);
                } else {
                    ht->drop(ht->buckets[i]->data);
                }
                free(ht->buckets[i]);
            }
        }
        }
    }
    free(ht->buckets);
    ht->buckets = NULL;
    free(ht);
    ht = NULL;
}

int hashtable_lookup(const hashtable *ht, const char *key, void **item, int *pos)
{
    int index;
    for (int i = 0; i < ht->capacity; i++) {
        index = (_h1(ht, key) + (i * _h2(ht, key))) % ht->capacity;
        /* Found available spot */
        if (ht->buckets[index] == NULL) {
            *pos = index;
            /* printf("Hash key '%s' to position %d\n", key, index); */
            return -1;
        /* key is already in the map */
        } else if (strcmp(ht->buckets[index]->key, key) == 0) {
            /* printf("Key '%s' is already in the map at %d\n", key, index); */
            *item = ht->buckets[index]->data;
            *pos = index;
            return 0;
        }
    }
    return -1;
}

void *hashtable_get(const hashtable *ht, const char *key) 
{
    void *tmp; 
    int pos;
    if (hashtable_lookup(ht, key, (void*) &tmp, &pos) == 0) {
        return tmp;
    }
    return NULL;
}

int hashtable_add(hashtable *ht, const char *key, const void *item, void (*drop)(void *item), void (*print)(void *item), const int flag)
{
    if (key == NULL || item == NULL) return -1;

    // expand funct, if fail to extends, return -1
    /* if (ht->size >= ht->capacity / 2) */
    // Now assume it has enough capacity
    if (ht->size >= ht->capacity / 2) {
        fprintf(stderr, "Reaching 1/2 capacity");
        exit(EXIT_FAILURE);
    }

    int pos = -1;
    void *ret = NULL;
    /* Key is already in the map */
    if (hashtable_lookup(ht, key, (void*) &ret, &pos) == 0) {
        if (flag == 1) { // override duplicate key
            fprintf(stderr, "%s is existed, overrided it\n", key);
            ht->drop(ht->buckets[pos]->data);
            ht->buckets[pos]->data = (void*) item;
            return 0;
        } else if (flag == 2) {
            /* ht->drop((void*)item); */
            /* fprintf(stderr, "%s is nothing is added, overrided it", key); */
            // do nothing
        }

        return 1;
    }

    if (pos != -1) {
        entry *e = malloc(sizeof(*e));
        e->key = strdup(key);
        e->data = (void *) item;
        e->drop = drop == NULL ? NULL : drop;
        e->print = print == NULL ? NULL : print;
        ht->buckets[pos] = e;
        ht->size++;
        return 0;
    }

    return -1;
}

int hashtable_remove(hashtable *ht, const char* key, void **item)
{
    int pos = -1;
    if (hashtable_lookup(ht, key, item, &pos) == 0) {
        free(ht->buckets[pos]->key);        
        ht->buckets[pos]->key = NULL;        
        free(ht->buckets[pos]);
        ht->buckets[pos] = NULL;
        ht->size--;
        return 0;
    }
    return -1;
}

void hashtable_print(hashtable *ht, void (*print)(const void*data))
{
    if (ht->size == 0) return;

    /* Look up for last item in the map, this for terminate last comma */
    int i, last = 0;
    for (i = 0; i < ht->capacity; i++) {
        if (ht->buckets[i] != NULL) last = i;
    }

    printf("{\n");
    for (i = 0; i < ht->capacity; i++) {
        if (ht->buckets[i] != NULL) { 
            printf("  \"%s\": ", ht->buckets[i]->key);
            /* printf("  (%d) \"%s\": ", i, ht->buckets[i]->key); */ /* Print index */
            if (ht->buckets[i]->print != NULL) {
                ht->buckets[i]->print(ht->buckets[i]->data);
            } else {
                print(ht->buckets[i]->data);
            }
            printf("%s", i == last ? "" : ",\n");
        }
    }
    printf("\n}");
}
