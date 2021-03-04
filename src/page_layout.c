#include<page_layout.h>

#include<tuple.h>

#include<bitmap.h>

#include<string.h>
#include<stdlib.h>

// get page_layout to use for the given tuple definition
page_layout get_page_layout_type(const tuple_def* tpl_d)
{
	// for variable sized tuples use SLOTTED_PAGE_LAYOUT
	// else use FIXED_ARRAY_PAGE_LAYOUT for fixed length tuples
	if(tpl_d->size == VARIABLE_SIZED)
		return SLOTTED_PAGE_LAYOUT;
	else
		return FIXED_ARRAY_PAGE_LAYOUT;
}

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

uint16_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	const uint16_t* count = page + get_tuple_count_offset();
	return *count;
}

int insert_tuple_at(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
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
		default :
		{
			return 0;
		}
	}
}

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(!can_accomodate_tuple(page, page_size, tpl_d, external_tuple))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
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
		default :
		{
			return 0;
		}
	}
}

int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			char* is_valid = page + get_bitmap_offset_FIXED_ARRAY();

			// indexed tuple does not exist, so can not delete it
			if(!get_bit(is_valid, index))
				return 0;

			reset_bit(is_valid, index);

			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			const char* is_valid = page + get_bitmap_offset_FIXED_ARRAY();

			return get_bit(is_valid, index);
		}
		default :
		{
			return 0;
		}
	}
}

void* seek_to_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return NULL;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return NULL;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			const void* tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

			return (void*)(tuples + (index * tpl_d->size));
		}
		default :
		{
			return NULL;
		}
	}
}

int can_accomodate_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

			const uint16_t* count = page + get_tuple_count_offset();

			return (*count) < capacity;
		}
		default :
		{
			return 0;
		}
	}
}

uint32_t get_capacity_for_tuple_at_index(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			return tpl_d->size;
		}
		default :
		{
			return 0;
		}
	}
}

uint32_t get_size_for_tuple_at_index(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			return tpl_d->size;
		}
		default :
		{
			return 0;
		}
	}
}

uint32_t get_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

			const uint16_t* count = page + get_tuple_count_offset();

			return (capacity - (*count)) * tpl_d->size;
		}
		default :
		{
			return 0;
		}
	}
}

int compact_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			// TODO
			return 0;
		}
		default :
		{
			return 0;
		}
	}
}

void print_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	char* print_buffer = malloc(tpl_d->size + (tpl_d->element_count * 32));

	uint16_t count = get_tuple_count(page, page_size, tpl_d);

	if(tpl_d->size == VARIABLE_SIZED)	// case : SLOTTED PAGE
		printf("SLOTTED PAGE     : size(%u) : tuples(%u)\n\n", page_size, count);
	else								// case : FIXED ARRAY PAGE
		printf("FIXED ARRAY PAGE : size(%u) : tuples(%u)\n\n", page_size, count);

	for(uint16_t i = 0; i < count; i++)
	{
		printf("\t Tuple %u\n", i);
		if(exists_tuple(page, page_size, tpl_d, i))
		{
			print_buffer[0] = '\0';
			sprint_tuple(print_buffer, seek_to_nth_tuple(page, page_size, tpl_d, i), tpl_d);
			printf("\t\t %s\n\n", print_buffer);
		}
		else
			printf("\t\t %s\n\n", "DELETED");
	}
	printf("\n\n\n");

	free(print_buffer);
}