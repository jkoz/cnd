#ifndef _HT_H
#define _HT_H

#include <stdbool.h>
#include <stddef.h>

/* 
 * Hash table entry (slot may be filled or empty).
 *	- key is NULL if this slot is empty
 */
typedef struct {
    const char* key;  
    void* value;
} ht_entry;

// Hash table structure: create with ht_create, free with ht_destroy.
typedef struct {
    ht_entry* entries;  // hash slots
    size_t capacity;    // size of _entries array
    size_t length;      // number of items in hash table
    void (*destroy)(void *data); // destroy cb individual value
} ht;

/* 
 * Iterator for hash table
 */
typedef struct {
    const char* key;  // current key
    void* value;      // current value
    const ht* _table; // reference to hash table being iterated
    size_t _index;    // current index into ht._entries
} hti;

/*
 * Initialized a hash table
 *  - capacity: initial avaible address spaces in the table
 *  - destroy: function will be called to clean up
 */
ht* ht_init(int capacity, void (*destroy)(void *data));

// Free memory allocated for hash table, including allocated keys.
void ht_destroy(ht* table);

// Get item with given key (NUL-terminated) from hash table. Return
// value (which was set with ht_set), or NULL if key not found.
void* ht_get(ht* table, const char* key);

// Set item with given key (NUL-terminated) to value (which must not
// be NULL). If not already present in table, key is copied to newly
// allocated memory (keys are freed automatically when ht_destroy is
// called). Return address of copied key, or NULL if out of memory.
const char* ht_set(ht* table, const char* key, void* value);

// Return number of items in hash table.
size_t ht_length(ht* table);

// Return new hash table iterator (for use with ht_next).
hti ht_iterator(ht* table);

// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call ht_set during iteration.
bool ht_next(hti* it);

void ht_print(ht *m, char* (*ts)(void *data));

#endif // _HT_H
