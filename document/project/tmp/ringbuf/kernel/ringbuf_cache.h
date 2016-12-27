#ifndef _RINGBUF_CACHE_H_
#define _RINGBUF_CACHE_H_

#include <linux/types.h>

#include "ringbuf_mgmt.h"

#define RB_CACHE_SIZE 600
/*
  Define the read cache type
*/
typedef struct {
uint32_t read_pos;
uint32_t write_pos;
char     buf[RB_CACHE_SIZE];
uint32_t last_cached_rb_pos;      /* The position in ringbuf for the last
                                     cached character.
                                     This variable is reset to the wrap counter
                                     within ringbuf when the anyone writes over
                                     the last cached position (i.e. cache/rb
                                     sync). */
uint32_t last_cached_rb_wrap_cnt; /* The number of times the cache filling
                                     from ringbuf has passed the last position
                                     of the ringbuf area.
                                     This variable is reset to the wrap counter
                                     within ringbuf when the anyone writes over
                                     the last cached position (i.e. cache/rb
                                     sync). */
int      is_full;
int      is_empty;
int      valid_cached_position;    /* False until the first valid value for
                                      last_cached_rb_wrap_cnt and
                                      last_cached_rb_pos has been written */
} Cache;

/* Initialize the cache to be empty */
void init_cache(Cache* p_cache);

/* Empty the cache. In contrast to init_cache, 
   the info of the last cached (read) positions in ringbuf
   is not cleared in empty_cache. */
void empty_cache(Cache* p_cache);

/*keep the data but restart the read positions*/
void restart_cache(Cache* p_cache);

/* Return number of chars in the cache */
uint32_t cached_chars(Cache* p_cache);

/* Add a string to the cache */
uint32_t add_to_cache(Cache* p_cache, const char str[], uint32_t num_chars);

/* Read and remove from the cache */
uint32_t get_from_cache(Cache* p_cache, char str[], uint32_t num_chars);

/* Read from cache but don't remove the read data from the cache */
uint32_t look_in_cache(Cache* p_cache, char str[], uint32_t num_chars);

/* 
   Look into the data cache and see if a real raw ringbuf header is first
   in the cache, If a valid header is first in the cache, 1 is returned,
   otherwize 0 is returned. In case of a valid header, the date and
   tag numeric is returned in parameters date and tag. However if
   the pointers are 0 of course nothing is returned in them.
*/
int examine_raw_header_if_first_in_cache(Cache* p_data_cache, uint32_t* date_num, uint32_t* tag_num);

/* 
   Like the function above, but this function also returns true (1)
   if there is fewer characters than a raw header, but the characters that
   are there looks like the beginning of header (shortened).
   If this function returns true, we should fill more in the cache until
   we know if the full raw header is coming or not.
*/
int potential_raw_header_if_first_in_cache(Cache* p_data_cache);

/* Look into the data cache. If there is a valid header first in that
   cache, remove the header from the data cache, translate it into the
   desired textual header string according to device_type and put the
   translated header in the translate cache.
   If the header was "raw" it is still moved, but untranslated. A "simple"
   head is just removed but not put into the translate cache.
   If a new header is found, *tag_in_header returns the tag in the
   found header. 
   If a header is found and 1 is returned, otherwize 0 is
   returned.

   Note that you have to provide one function that converts ringbuf
   "raw time" into local time string, and another function that 
   converts tag number into a tag string.
*/
int translate_if_raw_header_first(Cache* p_data_cache, 
                                  Cache* p_translate_cache,
                                  int (*get_local_time)(char *date_string, 
                                                        uint32_t raw_time),
                                  char* (*get_tag_string)(int tag_no),
                                  Device_type device_type,
                                  int* tag_in_header);

/* Fill the cache with data.
   You have to provide the data getter funtion (i.e. get_data).
   The parameter p_cache to the getter function is only needed internally in
   the driver implementation. 

   Parameter max_chars_to_add is used to get an option to only put a limited
   number of new characters to the cache (typically set to 1, if 
   get_data function blocks, and we want to examine characters one by one).
   If max_chars_to_add equals 0, or is greated than the unused space in
   the cache, data is read, using "get_data" function until the cache is
   filled.

   The parameter extra_param_fwd is just an extra pointer forwarded to the
   getter function. It could for instance be a pointer to a ringbuf structure,
   or a pointer to an open file.
*/
void fill_cache(Cache* p_cache, uint32_t max_chars_to_add,
                void* extra_param_fwd,
                uint32_t (*get_data)(Cache* p_cache,char  str[],
                                     uint32_t num_chars, void* extra_param));

#endif
