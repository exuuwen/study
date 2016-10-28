#ifndef _LIB_MAP_H
#define _LIB_MAP_H

typedef struct Map* Map;
typedef void  (*delete_method)(void *);

#define Key int



/* Get rid of "incompatible pointer" warning: */
#define lib_map_peek(map, key, return_datum)   lib_map_peek_(map, key, (void**)return_datum)
#define lib_map_destroy(map, datum_delete)     lib_map_destroy_(map, (delete_method)datum_delete)
#define lib_map_remove(map, key, datum_delete) lib_map_remove_(map, key, (delete_method)datum_delete)

Map     lib_map_create  (void);
void    lib_map_destroy_(Map map, delete_method datum_delete);
int     lib_map_size    (const struct Map* m);

void    lib_map_insert  (Map map, Key key, void* datum);
int     lib_map_remove_ (Map map, Key key, delete_method datum_delete);

void*   lib_map_find    (const struct Map* map, Key key);
int     lib_map_exists  (const struct Map* map, Key key);

void    lib_map_domain  (const struct Map* map, Key** ret_keys, int* ret_size);

void    lib_map_process (Map map, int(*callback_fn_p)(Key, void*, int),
                         int value, delete_method datum_delete);

struct MapIterator {
    struct {
        const struct entry* p;
        struct entry** bucket;
        struct entry** end;
    } priv;
    int end;
    int key;
    void* val;
};

struct MapIterator lib_map_Iterator(const struct Map*);
void lib_map_next(struct MapIterator*);

#undef Key

#endif


