#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<stdint.h>

#include<tuple_def.h>

// NONE OF THE FOLLOWING FUNCTIONS CAN BE CALLED ON AN EMPTY TUPLE
// i.e. please check true for (0 == is_empty_tuple_def(const tuple_def* tpl_d))
// prior to calling any of the following functions below




typedef enum page_layout page_layout;
enum page_layout
{
	SLOTTED_PAGE_LAYOUT,
	FIXED_ARRAY_PAGE_LAYOUT,
};

// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_def* tpl_d);




// returns the number of tuples in the page (including the deleted ones)
uint16_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d);




// insert and delete operation return 1 upon success, else they return 0 for failure
// to insert a tuple at the end in the given page
int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// insert/update tuple at the specified index,
// index must be < get_tuple_count()
// if ((index >= get_tuple_count()) || capacity_at_index < incomming_tuple_size), insert fails with 0
int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple);

// to remove a tuple at the given index in the page
// if index >= get_tuple_count(), delete fails with 0
int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);

// to check if a tuple at the given index in the page exists
// 1 means the tuple exists, else if 0 then the tuple does not exists
// it returns 0, also when the tuple index is out of bounds, i.e. (index >= get_tuple_count())
// or if it is deleted
int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);




// returns NULL on failure, when the tuple index is out of bounds, i.e. (index >= get_tuple_count())
// returns pointer to nth tuple in the page
void* seek_to_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);




// returns true if the given page has enough space to accomodate the given tuple at the end of the page
// a true from this function signifies that the insert_tuple() will not fail
int can_accomodate_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);




// returns total free space inside a given page
// free_space = total_space - space occupied by the tuples (including the deleted one's)
uint32_t get_free_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);




// prints all tuples in the page including the deleted ones with their tomb stones
void print_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d);

#endif

/*
**
*****************************************************************************************
********			PAGE FORMATS
*****************************************************************************************
**
**					SLOTTED PAGE
**
**		* CASE ::: tuple_definition->size == VARIABLE_SIZED page
**
**		* the first uint16_t equals the total number of tuples in the page.
**			(the deleted tuples are not included here)
**
**		* if there are N tuples in an SLOTTED_PAGE, 
**			then there are N uint16_t integers that give us pointer offsets in the page to 
**			the first addresses of N variable sized tuples in the page.
**		i.e.
**			void* page = PAGE_ADDRESS;
**
**			uint16_t Num_tuples = *((uint16_t)page);
**
**			uint16_t* Tuple_offsets = page + sizeof(Num_tuples);
**									OR 
**			uint16_t* Tuple_offsets = page + sizeof(uint16_t);
**
**			now the n th tuple =>	(consider n < N)
**
**				void* nth_tuple = page + Tuple_offsets[n];
**
**		* NOTE : all the Tuple_offsets are always ordered in their increasing order
**			except when the corresponding tuple is deleted.
**			for a deleted tuple at index i Tuple_offsets[i] = 0x0000
**
****************************************************************************************
**
**					FIXED_ARRAY PAGE
**
**		* CASE ::: tuple_definition->size != VARIABLE_SIZED page
**
**		* The first uint16_t equals the total number of tuples in the page.
**			(including the deleted tuples)
**
**		* Due to the fixed length of each tuple we can precompute the maximum number
**			of tuples that a page can accomodate, as:
**
**			= MAX_TUPLES_ACCOMODATABLE
**
**				= TOTAL_UNUSED_BITS_IN_PAGE / TOTAL_BITS_IN_A_TUPLE
**
**				= floor_function( ((PAGE_SIZE - 2) * 8) / ((tuple_size * 8) + 1) )
**
**			here, tuple_size = tuple_definition->size
**
**		* The 1 additional bit is required for marking the tombstones for each of the tuples.
**
**			total_bitmap_size (in bytes) = ceil_function(MAX_TUPLES_ACCOMODATABLE / 8)
**
**			0 -> tuple is deleted OR tuple does not exists
**			1 -> the tuple exists and is valid
**
**		* This bitmap of total_bitmap_size bytes forms the prefix of the page,
**			since due to fixed sized tuples.
**
**		* The rest of the page memory of (page_size - total_bitmap_size) bytes
**			is used as an array of tuples, each of (tuple_definition->size) bytes.
*****************************************************************************************
**
*/