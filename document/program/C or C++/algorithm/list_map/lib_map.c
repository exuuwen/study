#ifdef LINUX /* Linux kernel */
#include <linux/slab.h>
#define assert(x)
#else
#include <stdlib.h>
#include <assert.h>
#endif

#include "lib_memory.h"
#include "lib_map.h"


#define LIBMAP      "Map"
#define LIBMAPENTRY "MapEntry"
#define LIBMAPTABLE "MapTable"

#define InitTableSize 100   /* Initial size of hash table. Will grow if necessary.                          */
#define RehashLimit   0.69  /* = ln(1/2), i.e. rehash when there are > 50% chance of hitting a used slot.   */
#define RehashGrow    2     /* Grow the table X times when limit is reached.  */

typedef void* voidp;
#define Datum voidp   

typedef int Key;

/* Entry:
*/
typedef struct entry* entry;
struct entry {
    Key     key;
    Datum   datum;
    entry   next;
};

/* Map:
*/
struct Map {
    entry*  table;
    int     table_size;
    int     nof_entries;
    int     limit;
};

/*********************************************************************/

static __inline__ int  Key_eq(Key val1, Key val2)
{
    return val1 == val2;
}

/* Parametrize the hashing:
*/
static int isqrt(int n) 
{
    int i, sq;
    for (i = 0, sq = 0; sq < n; i++, sq += 2*i+1);
    
    return i; 
}

static int is_prime(int n) 
{
    int N = isqrt(n);
    int i;
    for (i = 2; i <= N; i++)
        if (n % i == 0)
            return 0;

    return 1; 
}

static int determine_table_size(int min_size) 
{
    while (!is_prime(min_size))
        min_size++;

    return min_size; 
}

static __inline__ int hash(Key key, int size)
{
    return (unsigned)key % size;
}

/*********************************************************************/

static entry entry_new(Key key, Datum datum, entry next)
{
    entry tmp = lib_malloc(sizeof(struct entry), LIBMAPENTRY);
    assert(tmp);

    tmp->key   = key;
    tmp->datum = datum;
    tmp->next  = next;

    return tmp; 
}

static void entry_delete(entry ent)    /* (does *not* free recursivly) */
{
    lib_free(ent, sizeof(struct entry), LIBMAPENTRY);
}

/*********************************************************************/
static int remove_from_chain(entry* entryRef, Key key, delete_method datum_delete)
{
    entry  e = *entryRef;
    entry* p = entryRef;
    
    while (e) 
    {
        if (Key_eq(e->key, key))
        {
            /* Unlink entry from chain */
            *p = e->next;
            /* Delete entry */
            if (datum_delete != NULL)
                datum_delete(e->datum);
            entry_delete(e);
            return 1;
        }
        /* Get next entry and keep reference to previous e->next */
        p = &(e->next);
        e = e->next;
    }

    return 0;
}
static void free_chain(entry ent, delete_method datum_delete)
{
    entry tmp;
    entry e;
    
    e = ent;
    while (e) 
    {
        if (datum_delete != NULL)
            datum_delete(e->datum);
        tmp = e->next; 
        entry_delete(e);
        e = tmp;
    }
}

static void insert_chain(entry ent, Map map)
{
    entry e = ent;
    while (e) 
    {
        lib_map_insert(map, e->key, e->datum);
        e = e->next;
    }
}

static __inline__ const struct entry* search(const struct entry* head, Key key)
{
    const struct entry* e = head;
    while (e) 
    {        
        if (e->key == key) 
        {
            break;
        }
        e = e->next;
    }

    return e;
}


static int  process_chain(entry *entryRef, int(*callback_fn_p)(Key, Datum, int), int value, delete_method datum_delete)
{
    entry  e = *entryRef;
    entry *p = entryRef;
    int    del = 0; /* Number of entries deleted */

    while (e)
    {
        /* The return values decides whether we should remove this entry. */
        if (1 == callback_fn_p(e->key, e->datum, value)) 
        {
            /* Unlink entry from chain */
            *p = e->next;
            /* Delete entry */
            if (datum_delete != NULL)
                datum_delete(e->datum);
            entry_delete(e);
            /* Get next entry. */
            e = *p;
            del++;
        }
        else 
        {
            /* Get next entry and keep reference to previous e->next */
            p = &(e->next);
            e = e->next;
        }
    }

    return del;
}


static void domain_of_chain(entry ent, Key* table, int* index)
{
    entry e = ent;

    while (e) 
    {
        table[*index] = e->key;
        *index += 1;
        e = e->next;
    }
}


static Map new_map(int init_table_size)
{
    int i;
    Map tmp = lib_malloc(sizeof(struct Map), LIBMAP);
    assert(tmp);

    tmp->table_size  = determine_table_size(init_table_size);
    tmp->table       = lib_malloc(tmp->table_size * sizeof(entry), LIBMAPTABLE);
    assert(tmp->table);
#ifdef LINUX
    tmp->limit       = (int)((tmp->table_size * 10) / 7); /* no floating-point numbers in Linux kernel */
#else
    tmp->limit       = (int)(tmp->table_size * RehashLimit);
#endif
    tmp->nof_entries = 0;
    for (i = 0; i < tmp->table_size; i++)
        tmp->table[i] = NULL;

    return tmp;
}

static void delete_map(Map map, delete_method datum_delete)   /* (does not free the 'map' struct) */
{
    int i;
    for (i = 0; i < map->table_size; i++)
        free_chain(map->table[i], datum_delete);
    lib_free(map->table, map->table_size * sizeof(entry), LIBMAPTABLE);
}

static void grow_table(Map map)
{
    Map n_map;
    int i;

    n_map = new_map(map->table_size * RehashGrow);
    for (i = 0; i < map->table_size; i++)
        insert_chain(map->table[i], n_map);

    delete_map(map, NULL);
    *map = *n_map;
    lib_free(n_map, sizeof(struct Map), LIBMAP);
}

/*********************************************************************/

Map lib_map_create(void)
{
    return new_map(InitTableSize);
}

/* Dispose a map. If non-NULL, datum_delete(void*) is called first for
 * each piece of data.
 */
void lib_map_destroy_(Map map, delete_method datum_delete)
{
    delete_map(map, datum_delete);
    lib_free(map, sizeof(struct Map), LIBMAP);
}


/* The number of entries in the map.
 */
int lib_map_size(const struct Map* map)
{
    return map->nof_entries;
}


/* Insert (key, datum) into 'map'.
 * Does not check for duplicates (but it is considered an error!).
 *
 * The sane semantics would be to overwrite the earlier entry, but
 * Insert() takes no datum_delete() function.
 *
 * The second best option would be to refuse the insert and return an
 * error, but that will break code which inserts duplicates and then
 * expects to find the new value.
 *
 * So we keep the current semantics: insert the duplicate, and let the
 * duplicate be the one found by Find() etc. Remove() will remove the
 * newest duplicate and "uncloak" the second-oldest.
 */
void lib_map_insert(Map map, Key key, Datum datum)
{
    int index = hash(key, map->table_size);
    map->table[index] = entry_new(key, datum, map->table[index]);
    map->nof_entries++;
    if (map->nof_entries >= map->limit)
        grow_table(map);
}


/* Remove the element with key 'key' if it exists, calling datum_delete(datum)
 * if 'datum_delete' is non-null.
 *
 * If there are duplicate 'key' entries (which shouldn't happen, but is the
 * user's responsibility!) only one is removed.
 *
 * Returns zero if 'key' was not found.
 */
int lib_map_remove_(Map map, Key key, delete_method datum_delete)
{
    int index = hash(key, map->table_size);
    int found = remove_from_chain(&map->table[index], key, datum_delete);
    if(found) 
    {
        map->nof_entries--;
    }

    return found;
}

/* Return the datum for key 'key', or NULL if it does not exist.
 */
void* lib_map_find(const struct Map* map, Key key)
{
    const struct entry* e = search(map->table[hash(key, map->table_size)], key);
    return e ? e->datum : NULL;
}


/* Return non-zero if 'key' exist.
 */
int lib_map_exists(const struct Map* map, Key key)
{
    const struct entry* e = search(map->table[hash(key, map->table_size)], key);
    return e != NULL;
}

void lib_map_domain(const struct Map* map, Key** ret_keys, int* ret_size)
{
    Key*    table;
    int     index = 0;
    int     i;

    table = lib_malloc(map->nof_entries * sizeof(Key), NULL);
    assert(table);
    
    for (i = 0; i < map->table_size; i++)
        domain_of_chain(map->table[i], table, &index);
    *ret_keys = table;
    *ret_size = index;

    assert(*ret_size == map->nof_entries);
}

void lib_map_process(Map map, int(*callback_fn_p)(Key, Datum, int), int value, delete_method datum_delete)
{
    int i, nof;
    for (i = 0; i < map->table_size; i++) 
    {
        nof = process_chain(&map->table[i], callback_fn_p, value, datum_delete);
        map->nof_entries -= nof;
    }
}

/*****************************************************************/
static void scan(struct MapIterator* const it)
{
    while (it->priv.bucket != it->priv.end && !*it->priv.bucket) 
    {
        it->priv.bucket++;
    }

    if (it->priv.bucket != it->priv.end) 
    {
        it->priv.p = *it->priv.bucket;
        it->end = 0;
        it->key = it->priv.p->key;
        it->val = it->priv.p->datum;
    }
    else 
    {
        it->end = 1;
    }
}

struct MapIterator lib_map_iterator(const struct Map* map)
{
    struct MapIterator it;
    it.priv.bucket = map->table;
    it.priv.end = map->table + map->table_size;

    it.priv.p = NULL;
    it.key = 0;
    it.val = NULL;

    scan(&it);

    return it;
}


/**
 * Advance a valid iterator to the next entry.
 * Does nothing if it has reached the end already.
 */
void lib_map_next(struct MapIterator* const it)
{
    if (it->end) return;

    it->priv.p = it->priv.p->next;
    if (it->priv.p) 
    {
        it->key = it->priv.p->key;
        it->val = it->priv.p->datum;
    }
    else 
    {
        it->priv.bucket++;
        scan(it);
    }
}

///////////////
#define TEST
#ifdef TEST
#include <stdio.h>

void delete_a(void *value)
{
    lib_free(value, sizeof(int), "test"); 
}

int process_a(Key key, void* value, int data)
{
    if (*(int*)value % data == 0)
        return 1;

    return 0;
}

int main()
{
    Map map = lib_map_create();
    printf("size is %d, table size %d\n", lib_map_size(map), map->table_size);
    
    int i; 
    int *a[202];

    for(i=201; i>=0; i--)
    {
        a[i] = lib_malloc(sizeof(int), "test");
        *a[i] = i;
        lib_map_insert(map, i, a[i]);
    }

    for(i=190; i<=202; i++)
    {
        void *t = lib_map_find(map, i);
        if (t)
            printf("key %d value %d\n", i, *((int*)t));
        else
            printf("key %d is not exist\n", i);
    }
    
    printf("size is %d, table size %d\n", lib_map_size(map), map->table_size);

    for(i=195; i<=202; i++)
    {
        int ret = lib_map_remove(map, i, delete_a);
        if (ret)
            printf("key %d delete ok\n", i);
        else
            printf("key %d is not exist\n", i);
    }

    printf("size is %d, table size %d\n", lib_map_size(map), map->table_size);

    lib_map_process(map, process_a, 2, delete_a);

    int *key;
    int size = 0;

    lib_map_domain(map, &key, &size);
    for(i=0; i<size; i++)
        printf("key[%d]:%d\n", i, key[i]);

    lib_free(key, 0, "tmp");

    printf("size is %d, table size %d\n", lib_map_size(map), map->table_size);

    struct MapIterator iter;
    iter = lib_map_iterator(map);
    
    while (!iter.end)
    {
        printf("key: %d, value:%d\n", iter.key, *(int*)iter.val);
        lib_map_next(&iter);
    }
     
    lib_map_destroy(map, delete_a);
    
    return 0;
}

#endif



 
