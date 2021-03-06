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

static inline uint32_t get_page_type_offset()
{
	return 0; 
}

static inline uint32_t get_reference_pages_count_offset()
{
	return 1; 
}

static inline uint32_t get_tuple_count_offset()
{
	return 2; 
}

static inline uint32_t get_reference_page_ids_offset()
{
	return 4; 
}

static inline uint32_t get_bitmap_offset_FIXED_ARRAY(const void* page)
{
	return get_reference_page_ids_offset() + (sizeof(uint32_t) * get_reference_pages_count(page)); 
}

static inline uint16_t get_tuple_capacity_FIXED_ARRAY(const void* page, uint32_t page_size, uint32_t tuple_size)
{
	return ((page_size - get_bitmap_offset_FIXED_ARRAY(page)) * 8) / (8 * tuple_size + 1); 
}

static inline uint32_t get_tuples_offset_FIXED_ARRAY(const void* page, uint32_t page_size, uint32_t tuple_size)
{
	uint16_t tuples_capacity = get_tuple_capacity_FIXED_ARRAY(page, page_size, tuple_size);
	return get_bitmap_offset_FIXED_ARRAY(page) + bitmap_size_in_bytes(tuples_capacity); 
}

static inline uint32_t get_tuple_offsets_offset_SLOTTED(const void* page)
{
	return get_reference_page_ids_offset() + (sizeof(uint32_t) * get_reference_pages_count(page)); 
}

// data_type size to use for storing tuple_offsets (1, 2 or 4)
static inline uint8_t get_size_of_tuple_offset_data_type_SLOTTED(uint32_t page_size)
{
	if(page_size <= (1<<8))
		return 1;
	else if(page_size <= (1<<16))
		return 2;
	else
		return 4;
}

// all tuple_offsets in a SLOTTED_PAGE_LAYOUT must be greater than or equal to this offset
static inline uint32_t get_free_space_offset_SLOTTED(const void* page, uint32_t page_size)
{
	return get_tuple_offsets_offset_SLOTTED(page) + 
		(get_size_of_tuple_offset_data_type_SLOTTED(page_size) * get_tuple_count(page));
}

// utility functions to get/set tuple offsets in a SLOTTED_PAGE_LAYOUT

// index < get_tuple_count()
static inline uint32_t get_tuple_offset_SLOTTED(const void* page, uint32_t page_size, uint32_t index)
{
	// INDEX OUT OF BOUNDS
	if(index >= get_tuple_count(page))
		return 0;

	const void* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED(page);

	switch(get_size_of_tuple_offset_data_type_SLOTTED(page_size))
	{
		case 1 :
		{
			const uint8_t* tuple_offsets_1 = tuple_offsets;
			return tuple_offsets_1[index];
		}
		case 2 :
		{
			const uint16_t* tuple_offsets_2 = tuple_offsets;
			return tuple_offsets_2[index];
		}
		case 4 :
		{
			const uint32_t* tuple_offsets_4 = tuple_offsets;
			return tuple_offsets_4[index];
		}

		// may never happen
		default :
			return 0;
	}
}

// index < get_tuple_count()
// return 1, on success and 0 on index OUT_OF_BOUNDS
static inline int set_tuple_offset_SLOTTED(void* page, uint32_t page_size, uint32_t index, uint32_t tuple_offset)
{
	// INDEX OUT OF BOUNDS
	if(index >= get_tuple_count(page))
		return 0;

	void* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED(page);

	switch(get_size_of_tuple_offset_data_type_SLOTTED(page_size))
	{
		case 1 :
		{
			uint8_t* tuple_offsets_1 = tuple_offsets;
			tuple_offsets_1[index] = tuple_offset;
			return 1;
		}
		case 2 :
		{
			uint16_t* tuple_offsets_2 = tuple_offsets;
			tuple_offsets_2[index] = tuple_offset;
			return 1;
		}
		case 4 :
		{
			uint32_t* tuple_offsets_4 = tuple_offsets;
			tuple_offsets_4[index] = tuple_offset;
			return 1;
		}

		// may never happen
		default :
			return 0;
	}
}

// -------------------------------------------
// -------------------------------------------

uint32_t get_minimum_page_size(uint8_t reference_pages_count)
{
	// constant_size = sizeof(page_type) + sizeof(references_pages_count) + sizeof(tuple_count) 
	// + reference_pages_count * sizeof(each_reference_page_id)
	return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + (reference_pages_count * sizeof(uint32_t));
}

int init_page(void* page, uint32_t page_size, uint8_t page_type, uint8_t reference_pages_count)
{
	if(page_size < get_minimum_page_size(reference_pages_count))
		return 0;

	uint8_t* page_type_p             = page + get_page_type_offset();
	uint8_t* reference_pages_count_p = page + get_reference_pages_count_offset();

	(*page_type_p)            = page_type;
	(*reference_pages_count_p) = reference_pages_count;

	return 1;
}

uint8_t get_page_type(const void* page)
{
	const uint8_t* page_type = page + get_page_type_offset();
	return (*page_type);
}

void set_page_type(void* page, uint8_t page_type)
{
	uint8_t* page_type_p = page + get_page_type_offset();
	(*page_type_p) = page_type;
}

uint8_t get_reference_pages_count(const void* page)
{
	const uint8_t* reference_pages_count = page + get_reference_pages_count_offset();
	return (*reference_pages_count);
}

uint16_t get_tuple_count(const void* page)
{
	const uint16_t* count = page + get_tuple_count_offset();
	return *count;
}

uint32_t get_reference_page_id(const void* page, uint8_t index)
{
	uint8_t reference_page_count = get_reference_pages_count(page);
	const uint32_t* reference_page_ids  = page + get_reference_page_ids_offset();
	return (index < reference_page_count) ? reference_page_ids[index] : 0;
}

int set_reference_page_id(void* page, uint8_t index, uint32_t page_id)
{
	uint8_t reference_page_count = get_reference_pages_count(page);
	uint32_t* reference_page_ids  = page + get_reference_page_ids_offset();
	if(index < reference_page_count)
	{
		reference_page_ids[index] = page_id;
		return 1;
	}
	return 0;
}

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t* count = page + get_tuple_count_offset();

			// size of tuple to be inserted
			uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

			// the index where this tuple will be inserted
			uint16_t index = (*count);

			// set valid offset for the new tuple, such that it is adjacent to the last tuple (or the end of the page)
			uint32_t new_tuple_offset;
			if(index == 0)
				new_tuple_offset = page_size - external_tuple_size;
			else
				new_tuple_offset = get_tuple_offset_SLOTTED(page, page_size, index - 1) - external_tuple_size;

			// this offset may not cross the new_free_space_offset
			// new_free_space_offset = free_space_offset after adding the new element's offset
			uint32_t new_free_space_offset = get_free_space_offset_SLOTTED(page, page_size) + get_size_of_tuple_offset_data_type_SLOTTED(page_size);
			if(new_free_space_offset > new_tuple_offset)
				return 0;

			// insert the new tuple_offset with the new value
			(*count) += 1;
			set_tuple_offset_SLOTTED(page, page_size, index, new_tuple_offset);

			// pointer to the new tuple in the page
			void* new_tuple_p = page + get_tuple_offset_SLOTTED(page, page_size, index);

			// move data from external tuple to the tuple in the page
			memmove(new_tuple_p, external_tuple, external_tuple_size);

			return 1;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			// if the tuple count of the page has reached the maximum capacity
			// then no tuples can be inserted further and the insert fails
			if(get_tuple_count(page) == get_tuple_capacity_FIXED_ARRAY(page, page_size, tpl_d->size))
				return 0;

			uint16_t* count = page + get_tuple_count_offset(page);
			char* is_valid  = page + get_bitmap_offset_FIXED_ARRAY(page);
			void* tuples    = page + get_tuples_offset_FIXED_ARRAY(page, page_size, tpl_d->size);

			// the index where this tuple will be inserted
			uint16_t index = (*count);

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
	if(index >= get_tuple_count(page))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t count = get_tuple_count(page);

			// size of tuple to be inserted
			uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

			// generate the new_offset_for_index with the largest possible offset
			// i.e. adjacent to the previous tuple (or the end of the page)
			uint32_t new_offset_for_index;
			if(index == 0)
				new_offset_for_index = page_size - external_tuple_size;
			else
				new_offset_for_index = get_tuple_offset_SLOTTED(page, page_size, index - 1) - external_tuple_size;

			// check if the new_offset does not over lap the succeeding tuple, if it has a succeeding tuple
			if(index < (count - 1))
			{
				// next tuple
				uint32_t next_tuple_offset = get_tuple_offset_SLOTTED(page, page_size, index + 1);
				const void* next_tuple = page + next_tuple_offset;
				uint32_t next_tuple_size = get_tuple_size(tpl_d, next_tuple);

				if(next_tuple_offset + next_tuple_size > new_offset_for_index)
					return 0;
			}
			else
			{
				// since this is the update to the last element, we make sure that
				// its offset may not cross the free_space_offset
				// new_free_space_offset = free_space_offset after adding the new element's offset
				uint32_t free_space_offset = get_free_space_offset_SLOTTED(page, page_size);
				if(free_space_offset > new_offset_for_index)
					return 0;
			}

			// update the offset of the tuple to be updated
			set_tuple_offset_SLOTTED(page, page_size, index, new_offset_for_index);

			// pointer to the new updated tuple in the page
			void* new_tuple_p = page + get_tuple_offset_SLOTTED(page, page_size, index);

			memmove(new_tuple_p, external_tuple, external_tuple_size);

			return 1;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			char* is_valid = page + get_bitmap_offset_FIXED_ARRAY(page);
			void* tuples   = page + get_tuples_offset_FIXED_ARRAY(page, page_size, tpl_d->size);

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
	if(index >= get_tuple_count(page))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint16_t* count     = page + get_tuple_count_offset();
			void* tuple_offsets = page + get_tuple_offsets_offset_SLOTTED(page);

			// move all the offsets after index to the front by 1 unit
			// and then set the last tuple offset to 0
			uint32_t tuple_offsets_to_copy = ((*count) - (index + 1));
			uint32_t size_of_tuple_offset_data_type = get_size_of_tuple_offset_data_type_SLOTTED(page_size);
			memmove(tuple_offsets + (index) * size_of_tuple_offset_data_type,
					tuple_offsets + (index + 1) * size_of_tuple_offset_data_type,
					tuple_offsets_to_copy * size_of_tuple_offset_data_type);

			set_tuple_offset_SLOTTED(page, page_size, ((*count) - 1), 0);

			// decrement the tuple count
			(*count)--;

			return 1;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			char* is_valid = page + get_bitmap_offset_FIXED_ARRAY(page);

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
	if(index >= get_tuple_count(page))
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
			const char* is_valid = page + get_bitmap_offset_FIXED_ARRAY(page);

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
	if(index >= get_tuple_count(page))
		return NULL;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint32_t tuple_offset_for_index = get_tuple_offset_SLOTTED(page, page_size, index);

			return (void*)(page + tuple_offset_for_index);
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			const void* tuples = page + get_tuples_offset_FIXED_ARRAY(page, page_size, tpl_d->size);

			return (void*)(tuples + (index * tpl_d->size));
		}
		default :
		{
			return NULL;
		}
	}
}

uint32_t get_free_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	uint16_t count = get_tuple_count(page);

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			if(count == 0)
				// (total page size) - (memory occupied for storing the header)
				return page_size - get_tuple_offsets_offset_SLOTTED(page);
			else
				// (offset of the last tuple) - (offset of the free space)
				return get_tuple_offset_SLOTTED(page, page_size, count - 1) - get_free_space_offset_SLOTTED(page, page_size);
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			uint16_t capacity = get_tuple_capacity_FIXED_ARRAY(page, page_size, tpl_d->size);

			return (capacity - count) * tpl_d->size;
		}
		default :
		{
			return 0;
		}
	}
}

void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	char* print_buffer = malloc(tpl_d->size + (tpl_d->element_count * 32));

	char* page_layout_type = "NONE";

	if(tpl_d->size == VARIABLE_SIZED)	// case : SLOTTED PAGE
		page_layout_type = "SLOTTED PAGE    ";
	else								// case : FIXED ARRAY PAGE
		page_layout_type = "FIXED ARRAY PAGE";


	uint8_t ref_count = get_reference_pages_count(page);
	uint16_t tup_count = get_tuple_count(page);

	printf("%s : size(%u) : reference_page_ids(%u) : tuples(%u",page_layout_type, page_size, ref_count, tup_count);
	if(tpl_d->size != VARIABLE_SIZED)	// case : FIXED ARRAY PAGE
		printf(" of %u)", get_tuple_capacity_FIXED_ARRAY(page, page_size, tpl_d->size));
	else 								// case : SLOTTED PAGE
		printf(") : tuple_offsets_data_type_size(%u)", get_size_of_tuple_offset_data_type_SLOTTED(page_size));
	printf("\n\n");

	for(uint8_t i = 0; i < ref_count; i++)
		printf("\t Reference page id [%u] : %u\n\n", i, get_reference_page_id(page, i));

	printf("\n");

	for(uint16_t i = 0; i < tup_count; i++)
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

void print_page_in_hex(const void* page, uint32_t page_size)
{
	for(int i = 0; i < page_size; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");
}