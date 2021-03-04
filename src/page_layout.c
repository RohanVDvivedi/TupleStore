#include<page_layout.h>

#include<tuple.h>

#include<bitmap.h>

#include<string.h>
#include<stdlib.h>

// -------------------------------------------
// UTILITY FUNCTIONS
// -------------------------------------------

static inline uint16_t get_tuple_capacity_FIXED_ARRAY(uint32_t page_size, uint32_t tuple_size)
{
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
	uint16_t tuples_capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tuple_size);
	return 2 + bitmap_size_in_bytes(tuples_capacity); 
}

static inline uint16_t get_tuple_offsets_offset_SLOTTED()
{
	return sizeof(uint16_t); 
}

// -------------------------------------------
// -------------------------------------------

int insert_tuple_at(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	if(tpl_d->size == VARIABLE_SIZED)
	{
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();
		void*     tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

		// indexed tuple has valid data
		if(get_bit(is_valid, index))
			return 0;

		void* new_tuple_p = tuples + (index * tpl_d->size);

		memmove(new_tuple_p, external_tuple, tpl_d->size);
		set_bit(is_valid, index);

		return 1;
	}
}

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(!can_accomodate_tuple(page, page_size, tpl_d, external_tuple))
		return 0;

	if(tpl_d->size == VARIABLE_SIZED)
	{
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE

		uint16_t* count    = page + get_tuple_count_offset();
		char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();
		void*     tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

		// calculate the index where this tuple can be inserted
		uint16_t index = (*count);

		// indexed tuple has valid data
		if(get_bit(is_valid, index))
			return 0;

		void* new_tuple_p = tuples + (index * tpl_d->size);

		memmove(new_tuple_p, external_tuple, tpl_d->size);
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

		uint16_t* count    = page + get_tuple_count_offset();
		char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();

		// check for index in bounds
		if(index >= (*count))
			return 0;

		// indexed tuple does not exist, so can not delete it
		if(!get_bit(is_valid, index))
			return 0;

		reset_bit(is_valid, index);

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

		const uint16_t* count    = page + get_tuple_count_offset();
		const char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();

		// check for index in bounds
		if(index >= (*count))
			return 0;

		return get_bit(is_valid, index);
	}
}

uint16_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	const uint16_t* count = page + get_tuple_count_offset();
	return *count;
}

uint32_t get_capacity_for_tuple_at_index(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
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
		return tpl_d->size;
	}
}

uint32_t get_size_for_tuple_at_index(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
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
		return tpl_d->size;
	}
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

		const uint16_t* count    = page + get_tuple_count_offset();
		const char*     is_valid = page + get_bitmap_offset_FIXED_ARRAY();
		const void*     tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

		// check for index in bounds
		if(index >= (*count))
			return NULL;

		// indexed tuple was deleted (! is_valid)
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

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		const uint16_t* count    = page + get_tuple_count_offset();

		return (*count) < capacity;
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

		uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

		const uint16_t* count    = page + get_tuple_count_offset();

		return (capacity - (*count)) * tpl_d->size;
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

		char* print_buffer = malloc(tpl_d->size + (tpl_d->element_count * 24));

		uint16_t count = get_tuple_count(page, page_size, tpl_d);
		printf("FIXED ARRAY PAGE : size(%u) : tuples(%u)\n\n", page_size, count);
		for(uint16_t i = 0; i < count; i++)
		{
			printf("\t Tuple %u\n", i);
			if(exists_tuple(page, page_size, tpl_d, i))
			{
				sprint_tuple(print_buffer, seek_to_nth_tuple(page, page_size, tpl_d, i), tpl_d);
				printf("\t\t %s\n\n", print_buffer);
			}
			else
				printf("\t\t %s\n\n", "DELETED");
		}
		printf("\n\n\n");

		free(print_buffer);
	}
}