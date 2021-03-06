#include <string.h>  /* memset */
#include <assert.h>

#include "kr_hashtable.h"

#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

typedef struct _kr_hashnode_t
{
    void *   key;
    void *   value;

    /* If key_hash == 0, node is not in use
     * If key_hash == 1, node is a tombstone
     * If key_hash >= 2, node contains data */
    unsigned int      key_hash;
}T_KRHashNode;

struct _kr_hashtable_t
{
    int               size;
    int               mod;
    unsigned int      mask;
    int               nnodes;
    int               noccupied;  /* nnodes + tombstones */
    T_KRHashNode       *nodes;
    KRHashFunc        hash_func;
    KREqualFunc       key_equal_func;
    volatile int      ref_count;  /*currently not used...*/
    KRDestroyNotify   key_destroy_func;
    KRDestroyNotify   value_destroy_func;
};

/* Each table size has an associated prime modulo (the first prime
 * lower than the table size) used to find the initial bucket. Probing
 * then works modulo 2^n. The prime modulo is necessary to get a
 * good distribution with poor hash functions. */
static const int prime_mod [] =
{
  1,          /* For 1 << 0 */
  2,
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,      /* For 1 << 16 */
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647  /* For 1 << 31 */
};

static void 
kr_hashtable_set_shift(T_KRHashTable *hash_table, int shift)
{
    int i;
    unsigned int mask = 0;
    
    hash_table->size = 1 << shift;
    hash_table->mod  = prime_mod[shift];
    
    for (i = 0; i < shift; i++)
    {
        mask <<= 1;
        mask |= 1;
    }
    
    hash_table->mask = mask;
}

static int 
kr_hashtable_find_closest_shift(int n)
{
    int i;

    for (i = 0; n; i++)
        n >>= 1;

    return i;
}

static void 
kr_hashtable_set_shift_from_size(T_KRHashTable *hash_table, int size)
{
    int shift;

    shift = kr_hashtable_find_closest_shift(size);
    shift = MAX(shift, HASH_TABLE_MIN_SHIFT);

    kr_hashtable_set_shift(hash_table, shift);
}

/*
 * kr_hashtable_lookup_node:
 * @hash_table: our #T_KRHashTable
 * @key: the key to lookup against
 * @hash_return: optional key hash return location
 * Return value: index of the described #T_KRHashNode
 *
 * Performs a lookup in the hash table.  Virtually all hash operations
 * will use this function internally.
 *
 * This function first computes the hash value of the key using the
 * user's hash function.
 *
 * If an entry in the table matching @key is found then this function
 * returns the index of that entry in the table, and if not, the
 * index of an empty node (never a tombstone).
 */
static inline unsigned int 
kr_hashtable_lookup_node(T_KRHashTable *hash_table, const void *key)
{
    T_KRHashNode *node;
    unsigned int node_index;
    unsigned int hash_value;
    unsigned int step = 0;
    
    /* Empty buckets have hash_value set to 0, and for tombstones, it's 1.
     * We need to make sure our hash value is not one of these. */
    hash_value = (* hash_table->hash_func)(key);
    if (hash_value <= 1)
        hash_value = 2;
    
    node_index = hash_value % hash_table->mod;
    node = &hash_table->nodes[node_index];
    
    while (node->key_hash)
    {
        /*  We first check if our full hash values
         *  are equal so we can avoid calling the full-blown
         *  key equality function in most cases.
         */
        if (node->key_hash == hash_value)
        {
            if (hash_table->key_equal_func)
            {
                if (hash_table->key_equal_func (node->key, key))
                    break;
            }
            else if (node->key == key)
            {
                break;
            }
        }
    
        step++;
        node_index += step;
        node_index &= hash_table->mask;
        node = &hash_table->nodes[node_index];
    }
    
    return node_index;
}

/*
 * kr_hashtable_lookup_node_for_insertion:
 * @hash_table: our #T_KRHashTable
 * @key: the key to lookup against
 * @hash_return: key hash return location
 * Return value: index of the described #T_KRHashNode
 *
 * Performs a lookup in the hash table, preserving extra information
 * usually needed for insertion.
 *
 * This function first computes the hash value of the key using the
 * user's hash function.
 *
 * If an entry in the table matching @key is found then this function
 * returns the index of that entry in the table, and if not, the
 * index of an unused node (empty or tombstone) where the key can be
 * inserted.
 *
 * The computed hash value is returned in the variable pointed to
 * by @hash_return. This is to save insertions from having to compute
 * the hash record again for the new record.
 */
static inline unsigned int
kr_hashtable_lookup_node_for_insertion (T_KRHashTable    *hash_table,
                                        const void     *key,
                                        unsigned int   *hash_return)
{
    T_KRHashNode *node;
    unsigned int node_index;
    unsigned int hash_value;
    unsigned int first_tombstone;
    kr_bool have_tombstone = FALSE;
    unsigned int step = 0;
    
    /* Empty buckets have hash_value set to 0, and for tombstones, it's 1.
     * We need to make sure our hash value is not one of these. */
    hash_value = (* hash_table->hash_func)(key);
    if (hash_value <= 1)
        hash_value = 2;
    
    *hash_return = hash_value;
    
    node_index = hash_value % hash_table->mod;
    node = &hash_table->nodes[node_index];
    
    while (node->key_hash)
    {
        /*  We first check if our full hash values
         *  are equal so we can avoid calling the full-blown
         *  key equality function in most cases.
         */
        if (node->key_hash == hash_value)
        {
            if (hash_table->key_equal_func)
            {
                if (hash_table->key_equal_func(node->key, key))
                    return node_index;
            }
            else if (node->key == key)
            {
                return node_index;
            }
        }
        else if (node->key_hash == 1 && !have_tombstone)
        {
            first_tombstone = node_index;
            have_tombstone = TRUE;
        }
    
        step++;
        node_index += step;
        node_index &= hash_table->mask;
        node = &hash_table->nodes[node_index];
    }
    
    if (have_tombstone)
        return first_tombstone;
    
    return node_index;
}

/*
 * kr_hashtable_remove_node:
 * @hash_table: our #T_KRHashTable
 * @node: pointer to node to remove
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes a node from the hash table and updates the node count.
 * The node is replaced by a tombstone. No table resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 */
static void
kr_hashtable_remove_node (T_KRHashTable   *hash_table,
                          T_KRHashNode    *node,
                          kr_bool      notify)
{
    if (notify && hash_table->key_destroy_func)
        hash_table->key_destroy_func(node->key);
    
    if (notify && hash_table->value_destroy_func)
        hash_table->value_destroy_func(node->value);
    
    /* Erect tombstone */
    node->key_hash = 1;
    
    /* Be GC friendly */
    node->key = NULL;
    node->value = NULL;
    
    hash_table->nnodes--;
}

/*
 * kr_hashtable_remove_all_nodes:
 * @hash_table: our #T_KRHashTable
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes all nodes from the table.  Since this may be a precursor to
 * freeing the table entirely, no resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 */
static void
kr_hashtable_remove_all_nodes (T_KRHashTable *hash_table, kr_bool notify)
{
    int i;
    
    for (i = 0; i < hash_table->size; i++)
    {
        T_KRHashNode *node = &hash_table->nodes[i];
    
        if (node->key_hash > 1)
        {
            if (notify && hash_table->key_destroy_func)
                hash_table->key_destroy_func(node->key);
    
            if (notify && hash_table->value_destroy_func)
                hash_table->value_destroy_func(node->value);
        }
    }
    
    /* We need to set node->key_hash = 0 for all nodes - might as well be GC
     * friendly and clear everything */
    memset (hash_table->nodes, 0, hash_table->size * sizeof (T_KRHashNode));
    
    hash_table->nnodes = 0;
    hash_table->noccupied = 0;
}

/*
 * kr_hashtable_resize:
 * @hash_table: our #T_KRHashTable
 *
 * Resizes the hash table to the optimal size based on the number of
 * nodes currently held.  If you call this function then a resize will
 * occur, even if one does not need to occur.  Use
 * kr_hashtable_maybe_resize() instead.
 *
 * This function may "resize" the hash table to its current size, with
 * the side effect of cleaning up tombstones and otherwise optimizing
 * the probe sequences.
 */
static void
kr_hashtable_resize (T_KRHashTable *hash_table)
{
    T_KRHashNode *new_nodes;
    int old_size;
    int i;
    
    old_size = hash_table->size;
    kr_hashtable_set_shift_from_size(hash_table, hash_table->nnodes * 2);
    
    new_nodes = (T_KRHashNode *)kr_calloc(sizeof(T_KRHashNode)*hash_table->size);
    
    for (i = 0; i < old_size; i++)
    {
        T_KRHashNode *node = &hash_table->nodes[i];
        T_KRHashNode *new_node;
        unsigned int hash_val;
        unsigned int step = 0;
    
        if (node->key_hash <= 1)
            continue;
    
        hash_val = node->key_hash % hash_table->mod;
        new_node = &new_nodes[hash_val];
    
        while (new_node->key_hash)
        {
            step++;
            hash_val += step;
            hash_val &= hash_table->mask;
            new_node = &new_nodes[hash_val];
        }
    
        *new_node = *node;
    }
    
    kr_free(hash_table->nodes);
    hash_table->nodes = new_nodes;
    hash_table->noccupied = hash_table->nnodes;
}

/*
 * kr_hashtable_maybe_resize:
 * @hash_table: our #T_KRHashTable
 *
 * Resizes the hash table, if needed.
 *
 * Essentially, calls kr_hashtable_resize() if the table has strayed
 * too far from its ideal size for its number of nodes.
 */
static inline void
kr_hashtable_maybe_resize (T_KRHashTable *hash_table)
{
    int noccupied = hash_table->noccupied;
    int size = hash_table->size;

    if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) ||
        (size <= noccupied + (noccupied / 16)))
        kr_hashtable_resize(hash_table);
}

/**
 * kr_hashtable_new:
 * @hash_func: a function to create a hash value from a key.
 *   Hash values are used to determine where keys are stored within the
 *   #T_KRHashTable data structure. The kr_direct_hash(), kr_int_hash(),
 *   kr_int64_hash(), kr_double_hash() and kr_str_hash() functions are provided
 *   for some common types of keys.
 *   If hash_func is %NULL, kr_direct_hash() is used.
 * @key_equal_func: a function to check two keys for equality.  This is
 *   used when looking up keys in the #T_KRHashTable.  The kr_direct_equal(),
 *   kr_int_equal(), kr_int64_equal(), kr_double_equal() and kr_str_equal()
 *   functions are provided for the most common types of keys.
 *   If @key_equal_func is %NULL, keys are compared directly in a similar
 *   fashion to kr_direct_equal(), but without the overhead of a function call.
 *
 * Creates a new #T_KRHashTable with a reference count of 1.
 *
 * Return value: a new #T_KRHashTable.
 **/
T_KRHashTable*
kr_hashtable_new (KRHashFunc    hash_func, KREqualFunc   key_equal_func)
{
    return kr_hashtable_new_full(hash_func, key_equal_func, NULL, NULL);
}


/**
 * kr_hashtable_new_full:
 * @hash_func: a function to create a hash value from a key.
 * @key_equal_func: a function to check two keys for equality.
 * @key_destroy_func: a function to free the memory allocated for the key
 *   used when removing the entry from the #T_KRHashTable or %NULL if you
 *   don't want to supply such a function.
 * @value_destroy_func: a function to free the memory allocated for the
 *   value used when removing the entry from the #T_KRHashTable or %NULL if
 *   you don't want to supply such a function.
 *
 * Creates a new #T_KRHashTable like kr_hashtable_new() with a reference count
 * of 1 and allows to specify functions to free the memory allocated for the
 * key and value that get called when removing the entry from the #T_KRHashTable.
 *
 * Return value: a new #T_KRHashTable.
 **/
T_KRHashTable*
kr_hashtable_new_full (KRHashFunc       hash_func,
                       KREqualFunc      key_equal_func,
                       KRDestroyNotify  key_destroy_func,
                       KRDestroyNotify  value_destroy_func)
{
    T_KRHashTable *hash_table;
    
    hash_table = (T_KRHashTable *)kr_calloc(sizeof(T_KRHashTable));
    kr_hashtable_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
    hash_table->nnodes             = 0;
    hash_table->noccupied          = 0;
    hash_table->hash_func          = hash_func;
    hash_table->key_equal_func     = key_equal_func;
    hash_table->ref_count          = 1;
    hash_table->key_destroy_func   = key_destroy_func;
    hash_table->value_destroy_func = value_destroy_func;
    hash_table->nodes = kr_calloc(sizeof(T_KRHashNode)*hash_table->size);
    
    return hash_table;
}

/**
 * kr_hashtable_destroy:
 * @hash_table: a #T_KRHashTable.
 *
 * Destroys all keys and values in the #T_KRHashTable and decrements its
 * reference count by 1. If keys and/or values are dynamically allocated,
 * you should either free them first or create the #T_KRHashTable with destroy
 * notifiers using kr_hashtable_new_full(). In the latter case the destroy
 * functions you supplied will be called on all keys and values during the
 * destruction phase.
 **/
void
kr_hashtable_destroy (T_KRHashTable *hash_table)
{
    assert(hash_table != NULL);
    assert(hash_table->ref_count > 0);
    
    kr_hashtable_remove_all(hash_table);
    kr_free(hash_table->nodes);
    kr_free(hash_table);
}

/**
 * kr_hashtable_lookup:
 * @hash_table: a #T_KRHashTable.
 * @key: the key to look up.
 *
 * Looks up a key in a #T_KRHashTable. Note that this function cannot
 * distinguish between a key that is not present and one which is present
 * and has the value %NULL. If you need this distinction, use
 * kr_hashtable_lookup_extended().
 *
 * Return value: the associated value, or %NULL if the key is not found.
 **/
void *
kr_hashtable_lookup (T_KRHashTable   *hash_table, const void * key)
{
    T_KRHashNode *node;
    unsigned int      node_index;
    
    if (hash_table == NULL)
        return NULL;
    
    node_index = kr_hashtable_lookup_node(hash_table, key);
    node = &hash_table->nodes[node_index];
    
    return node->key_hash ? node->value : NULL;
}

/**
 * kr_hashtable_lookup_extended:
 * @hash_table: a #T_KRHashTable
 * @lookup_key: the key to look up
 * @orig_key: return location for the original key, or %NULL
 * @value: return location for the value associated with the key, or %NULL
 *
 * Looks up a key in the #T_KRHashTable, returning the original key and the
 * associated value and a #kr_bool which is %TRUE if the key was found. This
 * is useful if you need to free the memory allocated for the original key,
 * for example before calling kr_hashtable_remove().
 *
 * You can actually pass %NULL for @lookup_key to test
 * whether the %NULL key exists.
 *
 * Return value: %TRUE if the key was found in the #T_KRHashTable.
 **/
kr_bool
kr_hashtable_lookup_extended (T_KRHashTable    *hash_table,
                              const void    *lookup_key,
                              void       **orig_key,
                              void       **value)
{
    T_KRHashNode *node;
    unsigned int      node_index;
    
    if (hash_table == NULL)
        return FALSE;
    
    node_index = kr_hashtable_lookup_node(hash_table, lookup_key);
    node = &hash_table->nodes[node_index];
    
    if (!node->key_hash)
        return FALSE;
    
    if (orig_key)
        *orig_key = node->key;
    
    if (value)
        *value = node->value;
    
    return TRUE;
}

/*
 * kr_hashtable_insert_internal:
 * @hash_table: our #T_KRHashTable
 * @key: the key to insert
 * @value: the value to insert
 * @keep_new_key: if %TRUE and this key already exists in the table
 *   then call the destroy notify function on the old key.  If %FALSE
 *   then call the destroy notify function on the new key.
 *
 * Implements the common logic for the kr_hashtable_insert() and
 * kr_hashtable_replace() functions.
 *
 * Do a lookup of @key.  If it is found, replace it with the new
 * @value (and perhaps the new @key).  If it is not found, create a
 * new node.
 */
static void
kr_hashtable_insert_internal (T_KRHashTable *hash_table,
                              void        *key,
                              void        *value,
                              kr_bool      keep_new_key)
{
    T_KRHashNode *node;
    unsigned int node_index;
    unsigned int key_hash;
    unsigned int old_hash;
    
    assert(hash_table != NULL);
    assert(hash_table->ref_count > 0);
    
    node_index = kr_hashtable_lookup_node_for_insertion(hash_table, key, &key_hash);
    node = &hash_table->nodes[node_index];
    
    old_hash = node->key_hash;
    
    if (old_hash > 1)
    {
        if (keep_new_key)
        {
            if (hash_table->key_destroy_func)
                hash_table->key_destroy_func(node->key);
            node->key = key;
        }
        else
        {
            if (hash_table->key_destroy_func)
                hash_table->key_destroy_func(key);
        }
    
        if (hash_table->value_destroy_func)
            hash_table->value_destroy_func(node->value);
    
        node->value = value;
    }
    else
    {
        node->key = key;
        node->value = value;
        node->key_hash = key_hash;
      
        hash_table->nnodes++;
      
        if (old_hash == 0)
        {
            /* We replaced an empty node, and not a tombstone */
            hash_table->noccupied++;
            kr_hashtable_maybe_resize(hash_table);
        }
    }
}

/**
 * kr_hashtable_insert:
 * @hash_table: a #T_KRHashTable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #T_KRHashTable.
 *
 * If the key already exists in the #T_KRHashTable its current value is replaced
 * with the new value. If you supplied a @value_destroy_func when creating the
 * #T_KRHashTable, the old value is freed using that function. If you supplied
 * a @key_destroy_func when creating the #T_KRHashTable, the passed key is freed
 * using that function.
 **/
void
kr_hashtable_insert (T_KRHashTable *hash_table,
                     void        *key,
                     void        *value)
{
    kr_hashtable_insert_internal(hash_table, key, value, FALSE);
}

/**
 * kr_hashtable_replace:
 * @hash_table: a #T_KRHashTable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #T_KRHashTable similar to
 * kr_hashtable_insert(). The difference is that if the key already exists
 * in the #T_KRHashTable, it gets replaced by the new key. If you supplied a
 * @value_destroy_func when creating the #T_KRHashTable, the old value is freed
 * using that function. If you supplied a @key_destroy_func when creating the
 * #T_KRHashTable, the old key is freed using that function.
 **/
void
kr_hashtable_replace (T_KRHashTable *hash_table,
                      void        *key,
                      void        *value)
{
    kr_hashtable_insert_internal(hash_table, key, value, TRUE);
}

/*
 * kr_hashtable_remove_internal:
 * @hash_table: our #T_KRHashTable
 * @key: the key to remove
 * @notify: %TRUE if the destroy notify handlers are to be called
 * Return value: %TRUE if a node was found and removed, else %FALSE
 *
 * Implements the common logic for the kr_hashtable_remove() and
 * kr_hashtable_steal() functions.
 *
 * Do a lookup of @key and remove it if it is found, calling the
 * destroy notify handlers only if @notify is %TRUE.
 */
static kr_bool
kr_hashtable_remove_internal (T_KRHashTable    *hash_table,
                              const void     *key,
                              kr_bool        notify)
{
    T_KRHashNode *node;
    unsigned int node_index;
    
    if (hash_table == NULL)
        return FALSE;
    
    node_index = kr_hashtable_lookup_node(hash_table, key);
    node = &hash_table->nodes[node_index];
    
    /* kr_hashtable_lookup_node() never returns a tombstone, so this is safe */
    if (!node->key_hash)
        return FALSE;
    
    kr_hashtable_remove_node(hash_table, node, notify);
    kr_hashtable_maybe_resize(hash_table);
    
    return TRUE;
}

/**
 * kr_hashtable_remove:
 * @hash_table: a #T_KRHashTable.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #T_KRHashTable.
 *
 * If the #T_KRHashTable was created using kr_hashtable_new_full(), the
 * key and value are freed using the supplied destroy functions, otherwise
 * you have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Return value: %TRUE if the key was found and removed from the #T_KRHashTable.
 **/
kr_bool
kr_hashtable_remove (T_KRHashTable    *hash_table,
                     const void     *key)
{
    return kr_hashtable_remove_internal(hash_table, key, TRUE);
}

/**
 * kr_hashtable_steal:
 * @hash_table: a #T_KRHashTable.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #T_KRHashTable without
 * calling the key and value destroy functions.
 *
 * Return value: %TRUE if the key was found and removed from the #T_KRHashTable.
 **/
kr_bool
kr_hashtable_steal (T_KRHashTable    *hash_table,
                    const void *  key)
{
    return kr_hashtable_remove_internal(hash_table, key, FALSE);
}

/**
 * kr_hashtable_remove_all:
 * @hash_table: a #T_KRHashTable
 *
 * Removes all keys and their associated values from a #T_KRHashTable.
 *
 * If the #T_KRHashTable was created using kr_hashtable_new_full(), the keys
 * and values are freed using the supplied destroy functions, otherwise you
 * have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Since: 2.12
 **/
void
kr_hashtable_remove_all (T_KRHashTable *hash_table)
{
    assert(hash_table != NULL);

    kr_hashtable_remove_all_nodes(hash_table, TRUE);
    kr_hashtable_maybe_resize(hash_table);
}

/**
 * kr_hashtable_steal_all:
 * @hash_table: a #T_KRHashTable.
 *
 * Removes all keys and their associated values from a #T_KRHashTable
 * without calling the key and value destroy functions.
 *
 * Since: 2.12
 **/
void
kr_hashtable_steal_all (T_KRHashTable *hash_table)
{
    assert(hash_table != NULL);

    kr_hashtable_remove_all_nodes (hash_table, FALSE);
    kr_hashtable_maybe_resize (hash_table);
}

/*
 * kr_hashtable_foreach_remove_or_steal:
 * @hash_table: our #T_KRHashTable
 * @func: the user's callback function
 * @user_data: data for @func
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Implements the common logic for kr_hashtable_foreach_remove() and
 * kr_hashtable_foreach_steal().
 *
 * Iterates over every node in the table, calling @func with the key
 * and value of the node (and @user_data).  If @func returns %TRUE the
 * node is removed from the table.
 *
 * If @notify is true then the destroy notify handlers will be called
 * for each removed node.
 */
static unsigned int
kr_hashtable_foreach_remove_or_steal (T_KRHashTable *hash_table,
                                      KRHRFunc       func,
                                      void        *user_data,
                                      kr_bool      notify)
{
    unsigned int deleted = 0;
    int i;
    
    for (i = 0; i < hash_table->size; i++)
    {
        T_KRHashNode *node = &hash_table->nodes[i];
    
        if (node->key_hash > 1 && (* func)(node->key, node->value, user_data))
        {
            kr_hashtable_remove_node (hash_table, node, notify);
            deleted++;
        }
    }
    
    kr_hashtable_maybe_resize (hash_table);
    
    return deleted;
}

/**
 * kr_hashtable_foreach_remove:
 * @hash_table: a #T_KRHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each key/value pair in the #T_KRHashTable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #T_KRHashTable. If you supplied key or value destroy functions when creating
 * the #T_KRHashTable, they are used to free the memory allocated for the removed
 * keys and values.
 *
 * See #T_KRHashTableIter for an alternative way to loop over the 
 * key/value pairs in the hash table.
 *
 * Return value: the number of key/value pairs removed.
 **/
unsigned int
kr_hashtable_foreach_remove (T_KRHashTable *hash_table,
                             KRHRFunc     func,
                             void        *user_data)
{
    if (hash_table == NULL || func == NULL)
        return 0;

    return kr_hashtable_foreach_remove_or_steal(hash_table, func, user_data, TRUE);
}

/**
 * kr_hashtable_foreach_steal:
 * @hash_table: a #T_KRHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each key/value pair in the #T_KRHashTable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #T_KRHashTable, but no key or value destroy functions are called.
 *
 * See #T_KRHashTableIter for an alternative way to loop over the 
 * key/value pairs in the hash table.
 *
 * Return value: the number of key/value pairs removed.
 **/
unsigned int
kr_hashtable_foreach_steal (T_KRHashTable *hash_table,
                            KRHRFunc     func,
                            void        *user_data)
{
    if (hash_table == NULL || func == NULL)
        return 0;

    return kr_hashtable_foreach_remove_or_steal (hash_table, func, user_data, FALSE);
}

/**
 * kr_hashtable_foreach:
 * @hash_table: a #T_KRHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the
 * #T_KRHashTable.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * kr_hashtable_foreach_remove().
 *
 * See kr_hashtable_find() for performance caveats for linear
 * order searches in contrast to kr_hashtable_lookup().
 **/
void
kr_hashtable_foreach (T_KRHashTable *hash_table,
                      KRHFunc      func,
                      void        *user_data)
{
    assert(hash_table != NULL);
    assert(func != NULL);

    int i;    
    
    for (i = 0; i < hash_table->size; i++)
    {
        T_KRHashNode *node = &hash_table->nodes[i];
    
        if (node->key_hash > 1)
            (* func) (node->key, node->value, user_data);
    }
}

/**
 * kr_hashtable_find:
 * @hash_table: a #T_KRHashTable.
 * @predicate:  function to test the key/value pairs for a certain property.
 * @user_data:  user data to pass to the function.
 *
 * Calls the given function for key/value pairs in the #T_KRHashTable until
 * @predicate returns %TRUE.  The function is passed the key and value of
 * each pair, and the given @user_data parameter. The hash table may not
 * be modified while iterating over it (you can't add/remove items).
 *
 * Note, that hash tables are really only optimized for forward lookups,
 * i.e. kr_hashtable_lookup().
 * So code that frequently issues kr_hashtable_find() or
 * kr_hashtable_foreach() (e.g. in the order of once per every entry in a
 * hash table) should probably be reworked to use additional or different
 * data structures for reverse lookups (keep in mind that an O(n) find/foreach
 * operation issued for all n values in a hash table ends up needing O(n*n)
 * operations).
 *
 * Return value: The value of the first key/value pair is returned, for which
 * func evaluates to %TRUE. If no pair with the requested property is found,
 * %NULL is returned.
 *
 * Since: 2.4
 **/
void *
kr_hashtable_find (T_KRHashTable      *hash_table,
                   KRHRFunc          predicate,
                   void             *user_data)
{
    int i;
    
    if (hash_table == NULL||predicate == NULL)
        return NULL;
    
    for (i = 0; i < hash_table->size; i++)
    {
        T_KRHashNode *node = &hash_table->nodes [i];
    
        if (node->key_hash > 1 && predicate (node->key, node->value, user_data))
            return node->value;
    }
    
    return NULL;
}

/**
 * kr_hashtable_size:
 * @hash_table: a #T_KRHashTable.
 *
 * Returns the number of elements contained in the #T_KRHashTable.
 *
 * Return value: the number of key/value pairs in the #T_KRHashTable.
 **/
unsigned int
kr_hashtable_size (T_KRHashTable *hash_table)
{
    if (hash_table == NULL)
        return 0;

    return hash_table->nnodes;
}
