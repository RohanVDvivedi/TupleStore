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

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(!can_accomodate_tuple(page, page_size, tpl_d, external_tuple))
		return 0;

	// reference to the count of tuple on the page
	uint16_t* count = page + get_tuple_count_offset();

	// the index where this tuple will be inserted
	uint16_t index = (*count);

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED();

			// size of tuple to be inserted
			uint32_t external_tuple_size = get_tuple_size(tpl_d, exists_tuple);

			// set valid offset for the new tuple, such that it is adjacent to the last tuple (or the end of the page)
			uint16_t new_tuple_offset;
			if(index == 0)
				new_tuple_offset = page_size - external_tuple_size;
			else
				new_tuple_offset = tuple_offsets[index - 1] - external_tuple_size;

			// check for its overlap with the tuple offsets array
			if(get_tuple_offsets_offset_SLOTTED() + (*count) > new_tuple_offset)
				return 0;

			// update the tuple_offset with the new value
			tuple_offsets[index] = new_tuple_offset;

			// pointer to the new tuple in the page
			void* new_tuple_p = page + tuple_offsets[index];

			// move data from external tuple to the tuple in the page
			memmove(new_tuple_p, external_tuple, external_tuple_size);

			// increment the tuple counter on the page
			(*count) += 1;

			return 1;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			char* is_valid = page + get_bitmap_offset_FIXED_ARRAY();
			void* tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

			// pointer to the new tuple in the page
			void* new_tuple_p = tuples + (index * tpl_d->size);

			// move data from external tuple to the tuple in the page
			memmove(new_tuple_p, external_tuple, tpl_d->size);

			// set valid bit for the newly inserted tuple
			set_bit(is_valid, index);

			// increment the tuple counter on the page
			(*count) += 1;

			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t count = get_tuple_count(page, page_size, tpl_d);
			uint16_t* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED();

			// size of tuple to be inserted
			uint32_t external_tuple_size = get_tuple_size(tpl_d, exists_tuple);

			// generate the new_offset_for_index with the largest possible offset
			// i.e. adjacent to the previous tuple (or the end of the page)
			uint16_t new_offset_for_index;
			if(index == 0)
				new_offset_for_index = page_size - external_tuple_size;
			else
				new_offset_for_index = tuple_offsets[index - 1] - external_tuple_size;

			// check if the new_offset does not over lap the succeeding tuple, if it has a succeeding tuple
			if(index < (count - 1))
			{
				// next tuple
				const void* next_tuple = page + tuple_offsets[index + 1];
				uint32_t next_tuple_size = get_tuple_size(tpl_d, next_tuple);

				if(tuple_offsets[index + 1] + next_tuple_size > new_offset_for_index)
					return 0;
			}
			else
			{
				// check for overlap with tuple offsets array
				if(get_tuple_offsets_offset_SLOTTED() + count > new_offset_for_index)
					return 0;
			}

			tuple_offsets[index] = new_offset_for_index;

			void* new_tuple_p = page + tuple_offsets[index];

			memmove(new_tuple_p, external_tuple, tpl_d->size);

			return 0;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			char* is_valid = page + get_bitmap_offset_FIXED_ARRAY();
			void* tuples   = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

			void* new_tuple_p = tuples + (index * tpl_d->size);

			// copy external_tuple to the new_tuple (in the page)
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

int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index OUT_OF_BOUNDS
	if(index >= get_tuple_count(page, page_size, tpl_d))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t* count         = page + get_tuple_count_offset();
			uint16_t* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED();

			// move all the offsets after index to the front by 1 unit
			// the last tuple offset is set to 0
			memmove(tuple_offsets + index, tuple_offsets + index + 1, 
				((*count) - (index + 1)) * sizeof(uint16_t));

			tuple_offsets[(*count) - 1] = 0;

			// decrement the tuple count
			(*count)--;

			return 1;
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
			// if the tuple index is in range
			// then the tuple exists
			return 1;
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
			const uint16_t* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED();

			return (void*)(page + tuple_offsets[index]);
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			const void* tuples = page + get_tuples_offset_FIXED_ARRAY(page_size, tpl_d->size);

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
	uint32_t free_space_in_page = get_free_space_in_page(page, page_size, tpl_d);

	uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

	return external_tuple_size <= free_space_in_page;
}

uint32_t get_free_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	uint16_t count = get_tuple_count(page, page_size, tpl_d);

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			const uint16_t* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED();

			if(count == 0)
				// (total page size) - (memory occupied for storing the tuple count)
				return page_size - get_tuple_count_offset();
			else
				// (offset of the last tuple) - (offset of the free space)
				return tuple_offsets[count-1] - (get_tuple_offsets_offset_SLOTTED() + count);
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page_size, tpl_d->size);

			return (capacity - count) * tpl_d->size;
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