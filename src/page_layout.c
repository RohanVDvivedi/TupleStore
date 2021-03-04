#include<page_layout.h>

#include<tuple.h>

#include<bitmap.h>

#include<string.h>

// -------------------------------------------
// UTILITY FUNCTIONS
// -------------------------------------------

static inline uint16_t get_tuple_capacity_FIXED_ARRAY(uint32_t page_size, uint32_t tuple_size)
{
	// unsigned int divisions give floor results by defaults
	return ((page_size - sizeof(uint16_t)) * 8) / (8 * tuple_size + 1); 
}

static inline uint16_t get_tuple_count_offset()
{
	return 0; 
}

static inline uint16_t get_bitmap_offset_FIXED_ARRAY()
{
	return sizeof(uint16_t); 
}

static inline uint16_t get_tuples_offset_FIXED_ARRAY(uint32_t page_size, uint32_t tuple_size)
{
	// unsigned int divisions give floor results by defaults
	uint16_t tuples_capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tuple_size);
	return 2 + bitmap_size_in_bytes(tuples_capacity); 
}

static inline uint16_t get_tuple_offsets_offset_SLOTTED()
{
	return sizeof(uint16_t); 
}

// -------------------------------------------
// -------------------------------------------

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		// check for index in bounds
		if(index >= capacity)
			return 0;

		uint16_t* count    = page + get_tuple_count_offset();
		char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();
		void*     tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

		// indexed tuple has valid data
		if(get_bit(is_valid, index))
			return 0;

		memmove(tuples + index * tpl_d->size, external_tuple, tpl_d->size);
		set_bit(is_valid, index);
		(*count) += 1;

		return 1;
	}
}

int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		// check for index in bounds
		if(index >= capacity)
			return 0;

		uint16_t* count    = page + get_tuple_count_offset();
		char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();

		// indexed tuple does not exist, so can not delete it
		if(!get_bit(is_valid, index))
			return 0;

		reset_bit(is_valid, index);
		(*count) -= 1;

		return 1;
	}
}

int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		// check for index in bounds
		if(index >= capacity)
			return 1;

		const char* is_valid = page + get_bitmap_offset_FIXED_ARRAY();

		return get_bit(is_valid, index);
	}
}

uint16_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	const uint16_t* count = page + get_tuple_count_offset();
	return *count;
}

void* seek_to_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return NULL;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		// check for index in bounds
		if(index >= capacity)
			return NULL;

		const char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();
		const void*     tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

		// indexed tuple has valid data
		if(!get_bit(is_valid, index))
			return NULL;

		return (void*)(tuples + (index * tpl_d->size));
	}
}

int can_accomodate_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

uint32_t get_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

int compact_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

void print_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
	}
}