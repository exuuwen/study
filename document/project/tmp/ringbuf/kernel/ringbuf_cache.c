#include <linux/string.h>

#include "ringbuf_cache.h"
#include "ringbuf_mgmt.h"
#include "ringbuf_utils.h"

void init_cache(Cache* p_cache)
{
	p_cache->write_pos = 0;
	p_cache->read_pos = 0;
	p_cache->is_full = 0;
	p_cache->is_empty = 1;
	p_cache->valid_cached_position = 0;
	p_cache->last_cached_rb_pos = 0xffffffff; /* Invalid initially */
	p_cache->last_cached_rb_wrap_cnt = 0;
}

void empty_cache(Cache* p_cache)
{
	p_cache->write_pos = 0;
	p_cache->read_pos = 0;
	p_cache->is_full = 0;
	p_cache->is_empty = 1;
}

void restart_cache(Cache* p_cache)
{
	p_cache->valid_cached_position = 0;
	p_cache->last_cached_rb_pos = 0xffffffff; /* Invalid initially */
	p_cache->last_cached_rb_wrap_cnt = 0;
}

uint32_t cached_chars(Cache* p_cache)
{
	if (p_cache->is_full)
		return RB_CACHE_SIZE;
	else if (p_cache->is_empty)
		return 0;
	else if (p_cache->write_pos > p_cache->read_pos)
		return p_cache->write_pos - p_cache->read_pos;
	else
		return RB_CACHE_SIZE - p_cache->read_pos + p_cache->write_pos;
}

uint32_t add_to_cache(Cache* p_cache, const char str[], uint32_t num_chars)
{
	uint32_t total_copied = 0;

	if (p_cache->is_full) 
		return 0;

	while (total_copied < num_chars) 
	{
		p_cache->buf[p_cache->write_pos++] = str[total_copied++];
		p_cache->is_empty = 0;/*TODO*/
		if (p_cache->write_pos >= RB_CACHE_SIZE) 
			p_cache->write_pos = 0;
		if (p_cache->write_pos == p_cache->read_pos) 
		{
	  		p_cache->is_full = 1;
	  		break;
		}
	}

	return total_copied;
}

uint32_t get_from_cache(Cache* p_cache, char str[], uint32_t num_chars)
{
	uint32_t total_copied = 0;

	if (p_cache->is_empty) 
		return 0;

	while (total_copied < num_chars) 
	{
		str[total_copied++] = p_cache->buf[p_cache->read_pos++];
		p_cache->is_full = 0;/*TODO*/
		if (p_cache->read_pos >= RB_CACHE_SIZE) 
			p_cache->read_pos = 0;
		if (p_cache->write_pos == p_cache->read_pos)
		{
	  		p_cache->is_empty = 1;
	  		break;
		}
	}

	return total_copied;
}

uint32_t look_in_cache(Cache* p_cache,char str[],uint32_t num_chars)
{
	uint32_t total_copied = 0;
	uint32_t read_pos = p_cache->read_pos;
	uint32_t write_pos = p_cache->write_pos;

	if (p_cache->is_empty) 
		return 0;

	while (total_copied < num_chars) 
	{
		str[total_copied++] = p_cache->buf[read_pos++];
		p_cache->is_full = 0;
		if (read_pos >= RB_CACHE_SIZE) 
			read_pos = 0;
		if (write_pos == read_pos)
			break;
	}

	return total_copied;
}

int examine_raw_header_if_first_in_cache(Cache* p_data_cache, uint32_t* date_num, uint32_t* tag_num)
{
	uint32_t act_read, matches, num1, num2;
	char str[RB_RAW_HEADER_SIZE + 1];
	char r_bracket;

	/* Optimization, fast initial check of first character match */
	//TODO check more
	act_read = look_in_cache(p_data_cache, str, 1);
	if (act_read == 0)
		return 0;
	else if (str[0] != RB_UNIQUE_RAW_HEADER_HEAD[0])
		return 0;

	/* Match for a complete header */
	act_read = look_in_cache(p_data_cache, str, RB_RAW_HEADER_SIZE);
	str[act_read] = '\0'; /* Null terminate */
	matches = sscanf(str, RB_UNIQUE_RAW_HEADER_HEAD"[%08X %02X%c", &num1, &num2, &r_bracket);

	if (date_num) 
		*date_num = num1;
	if (tag_num) 
		*tag_num = num2;

	return (matches == 3 && r_bracket == ']');
}

int potential_raw_header_if_first_in_cache(Cache* p_data_cache)
{
	uint32_t act_read;
	char str[RB_RAW_HEADER_SIZE + 1];

	/* Optimization, fast initial check of first character match */
	act_read = look_in_cache(p_data_cache, str, 1);
	if (act_read == 0)
		return 0;
	else if (str[0] != RB_UNIQUE_RAW_HEADER_HEAD[0])
		return 0;

	/* Match for more that looks like a header */
	act_read = look_in_cache(p_data_cache, str, RB_UNIQUE_RAW_HEADER_HEAD_SIZE + 1);
	str[act_read] = '\0'; /* Null terminate */

	return (strncmp(str, RB_UNIQUE_RAW_HEADER_HEAD"[", act_read) == 0);
}

int translate_if_raw_header_first(Cache* p_data_cache, 
                                  Cache* p_translate_cache,
                                  int (*get_local_time)(char *date_string, uint32_t raw_time),
                                  char* (*get_tag_string)(int tag_no),
                                  Device_type device_type,
                                  int* tag_in_header)
{
	uint32_t date_num, tag_num;
	char header[RB_MAX_FULL_HEADER_SIZE + 1]; /* Room for null char */
	char date_str[RB_MAX_DATE_SIZE*2 + 1]; /* Maybe different length depending on
		                                  different callback functions */

	if (!examine_raw_header_if_first_in_cache(p_data_cache, &date_num, &tag_num))
		/* No sense to continue, exit and leave tag_in_header untouched */
		return 0;

	*tag_in_header = tag_num;

	/* Remove the raw header */
	get_from_cache(p_data_cache, header, RB_RAW_HEADER_SIZE); 

	/* Just remove header for simple */
	/* return code should be 2 here.
	 * doShowRingBuf_3 uses it
	 * to determine where this function
	 * has returned*/
	if (is_simple(device_type)) 
		return 2; 

	/* Don't translate from raw device, just move it to the icept cache */
	/* return code should be 2 here.
	 * doShowRingBuf_3 uses it
	 * to determine where this function
	 * has returned*/
	if (is_raw(device_type)) 
	{
		add_to_cache(p_translate_cache, header, RB_RAW_HEADER_SIZE);
		return 2;
	}

	if (!is_full(device_type))
		printk(KERN_WARNING "Unexpected device type, not one of the defined enum types. Assuming <full>");

	get_local_time(date_str, date_num);
	snprintf(header, sizeof(header), "[%s %s]", date_str, get_tag_string(tag_num));
	add_to_cache(p_translate_cache, header, strlen(header));
	/* return code should be 1 here*/
	 /*doShowRingBuf_3 uses it
	 * to determine where this function
	 * has returned*/
	return 1;
}

void fill_cache(Cache* p_cache, uint32_t max_chars_to_add, void* extra_param_fwd,
                uint32_t (*get_data)(Cache* p_cache, char str[], uint32_t num_chars, void* extra_param))
{
	uint32_t act_read, left_to_read, num_to_read;
	
	char temp_str[100]; 

	if (max_chars_to_add == 0 || max_chars_to_add > (RB_CACHE_SIZE - cached_chars(p_cache)))
		left_to_read = RB_CACHE_SIZE - cached_chars(p_cache);
	else
		left_to_read = max_chars_to_add;

	while(left_to_read) 
	{
		if (left_to_read > sizeof(temp_str)) 
			num_to_read = sizeof(temp_str);
		else
			num_to_read = left_to_read;

		act_read = get_data(p_cache, temp_str, num_to_read, extra_param_fwd);
		add_to_cache(p_cache, temp_str, act_read);

		if (act_read < num_to_read || act_read  == 0) 
		{
	  		/* Next read might block, do not try to fill cache more here to be
		 	able to examine/print what we got in cache in the calling function. */
			return;
		}

		left_to_read -= act_read;
	}
}


