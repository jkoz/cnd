#ifndef _HASHTABLE_H
#define _HASHTABLE_H

typedef struct entry {
	char *key;
	void *data;
    void (*drop)(void *);
	void (*print)(void *);
} entry;

typedef struct _hashtable {
    int size;
    int capacity;
    entry **buckets;
	void (*drop)(void *item);
} hashtable;

/*
 * Initialize a hash table holding a array of void addresess
 * */
hashtable* hashtable_create(int capacity, void (*drop)(void *item));

/*
 * Destroy a hastable object
 * */
void hashtable_drop(hashtable *table);

/*
 * Insert an item to hashtable. If there already an item in the map, behaviour
 * will based on 'flag' 1 - override, old item will be dropped, 2 - ignored,
 * nothing is inserted to the map
 * */
int hashtable_add(hashtable *ht, const char *key, const void *item, void (*drop)(void *item), void (*print)(void *item), const int flag);

/*
 * Remove an item from hashtable
 * */
int hashtable_remove(hashtable *ht, const char* key, void **item);

/*
 * Print the table
 * */
void hashtable_print(hashtable *ht, void (*print)(const void*data));

/*
 * Lookup 'key' in the map, return location of its value in pos & item.
 *	0  - found the item for given key
 *	-1 - no item is under given key
 * */
int hashtable_lookup(const hashtable *ht, const char *key, void **item, int *pos);

void *hashtable_get(const hashtable *ht, const char *key);

#endif
