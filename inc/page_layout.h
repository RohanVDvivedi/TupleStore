#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<tuple_def.h>

#include<page_header.h>

typedef enum page_layout page_layout;
enum page_layout
{
	SLOTTED_PAGE_LAYOUT,
	FIXED_ARRAY_PAGE_LAYOUT,
};

// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_def* tpl_d);



// INITIALIZATION FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

// initialize page to start using it
int init_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d);

// returns a value that equals free space when tuple count = 0 
uint32_t get_space_to_be_allotted_for_all_tuples(uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

// returns the number of tuples in the page (including the deleted ones)
uint32_t get_tuple_count(const void* page, uint32_t page_size)



// INSERT DELETE and GET functions for tuples in the page

// to insert a tuple at the end in the given page, fails if the page is out of space
int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// returns 1, if the insert_tuple would succeed
int can_insert_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// inserts tuples from page_src starting with start_index and until end_index (or its tuple_count - 1), 
uint16_t insert_tuples_from_page(void* page, uint32_t page_size, const tuple_def* def, const void* page_src, uint16_t start_index, uint16_t last_index, int ignore_deleted);

// update tuple at the specified index, fails if the page is out of space, or if the index is out of bounds i.e. when index >= get_tuple_count()
int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple);

// to delete a tuple at the given index in the page, fails if index >= get_tuple_count()
int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);

// deletes all the tuple in the page
// for a slotted page it will also reset the end_of_free_space_offset
int delete_all_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d);

// to check if a tuple at the given index in the page exists
// returns 0, if the tuple was deleted OR if the index is out of bounds i.e. when index >= get_tuple_count())
int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);

// swap tuples at given indices i1 and i2
// return 0, if the swap fails
int swap_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t i1, uint16_t i2);

// returns pointer to nth tuple in the page, else returns NULL if exist_tuple fails
const void* get_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);



// PAGE COMPACTION and TOMB STONES removal functions

// returns 1, if the page_compaction is not a NOOP
// this function is a NOOP for FIXED_ARRAY_PAGE_LAYOUT
void run_page_compaction(void* page, uint32_t page_size, const tuple_def* tpl_d);

// discards all tomb stones on the page
void discard_all_deleted_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d);



// SPACE queries

// returns total free space left inside a given page, that can be used to accomodate tuples
uint32_t get_free_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns space_occupied by tuples on the page from start_index to last_index
uint32_t get_space_occupied_by_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t start_index, uint16_t last_index);

// equivalent to get_space_occupied_by_tuples[0, tuple_count - 1)
uint32_t get_space_occupied_by_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to get_space_allotted_to_all_tuples() - ( get_free_space_in_page() + get_space_occupied_by_all_tuples() )
uint32_t get_fragmentation_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);



// DEBUG FUNCTIONS

// prints all page_reference_ids andd all tuples in the page including the deleted
void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// prints page in hexadecimal format
void print_page_in_hex(const void* page, uint32_t page_size);

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
**		* the first uint8_t denotes the user defined page_type of the page.
**			* there are get_page_type() and set_page_type() function to access this user defined value.
**
**		* then comes uint8_t that denotes the number of reference page_ids, 
**			for furthering your search from this page.
**
**		* then comes uint16_t equals the total number of tuples in the page.
**			(the deleted tuples are not included here)
**
**		* then comes an array of uint32_t. each element is a reference page_id.
**			the length of this field is fixed for a given page upon initialization,
**			by the init_page() function.
**
** 		* then comes a uintK_t integer that gives the end of free_space offset in the page.
**			it is used to allocate more memory for new the tuples when inserted/updated.
**
**		* then, if there are N tuples in an SLOTTED_PAGE, 
**			then there are N uintK_t integers that give us offsets in the page to 
**			the first addresses of N variable sized tuples in the page.
**		i.e.
**			void* page = PAGE_ADDRESS;
**
**			uintK_t* Tuple_offsets = page + sizeof( all header fields prior to this field );
**
**			now the n th tuple =>	(consider n < N)
**
**				void* nth_tuple = page + Tuple_offsets[n];
**
**		* if the i-th tuple is deleted, then Tuple_offsets[i] == 0.
**
**		
**		struct page_of_SLOTTED_PAGE
**		{
**			uint8_t 	page_type;
**
**			uint8_t 	reference_page_count;
**
**			uint16_t 	tuple_count;
**
**			uint32_t 	reference_page_ids [ reference_page_count ] ;
**
**			uintK_t		end_of_free_space_offset;
**
**			uintK_t 	tuple_offsets [ tuple_count ];
**		}
**
**		if page_size is in range [1, 256] inclusive, then K = 8,
**		else if page_size is in range [257, 16536] inclusive, then K = 16,
** 		else K = 32.
**
****************************************************************************************
**
**					FIXED_ARRAY PAGE
**
**		* CASE ::: tuple_definition->size != VARIABLE_SIZED page
**
**		* the first uint8_t denotes the user defined page_type of the page.
**			* there are get_page_type() and set_page_type() function to access this user defined value.
**
**		* then comes uint8_t that denotes the number of reference page_ids, 
**			for furthering your search from this page.
**
**		* then comes uint16_t equals the total number of tuples in the page.
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
**
**
**		struct page_of_FIXED_ARRAY_PAGE
**		{
**			uint8_t 	page_type;
**
**			uint8_t 	reference_page_count;
**
**			uint16_t 	tuple_count;
**
**			uint32_t 	reference_page_ids [ reference_page_count ] ;
**
**			char*	 	is_valid_bitmap [ ceil_function( tuple_capacity / 8 ) ] ;
**
**			< tuples [ tuple_count ]; >
**		}
**
**
*****************************************************************************************
**
*/