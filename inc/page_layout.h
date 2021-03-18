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



// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint8_t reference_pages_count, const tuple_def* tpl_d, uint16_t tuple_count);



// to use any page, it must have been initialized
// the page_size provided must be greater than the get_minimum_page_size(*, *, ((tpl_d == NULL) ? 0 : 1))
// i.e. if you have a tuple definition for the page, thet you must be able to accomodate atleast a tuple worth of data on it
// The tuple_def parameter is not mandatory, you may pass NULL, if you are going to store only page references on the page
// if init_page fails, then it returns 0 (else 1 for success)
int init_page(void* page, uint32_t page_size, uint8_t page_type, uint8_t reference_pages_count, const tuple_def* tpl_d);

// getter and setter for the page_type that the user mentioned
uint8_t get_page_type(const void* page);
void set_page_type(void* page, uint8_t page_type);

// the total number of reference page count on this page
// i.e. this is the number of page_ids, you may want to follow to continue your search operation
uint8_t get_reference_pages_count(const void* page);

// returns the number of tuples in the page (including the deleted ones)
uint16_t get_tuple_count(const void* page);

// get_reference_page_id() returns the page_id that you can refer to, for your further search
// p_ref_id = index in the reference_page_ids array
// must => index < get_reference_page_count()
uint32_t get_reference_page_id(const void* page, uint8_t index);

// returns 1, if the reference_page_id at index was set to page_id in the reference_page_ids array
// returns 0 if index >= get_reference_page_count()
int set_reference_page_id(void* page, uint8_t index, uint32_t page_id);




// insert and delete operation return 1 upon success, else they return 0 for failure

// to insert a tuple at the end in the given page
// fails if the page is out of space
int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// returns 1, on success, if insert_tuple function would return with a success, without the actual insert happening
// i.e. for simplicity it returns get_free_space_in_page() >= get_size(external_tuple)
int can_accomodate_tuple_insert(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// inserts tuples from page_src starting with start_index and until end_index (or tuple_count - 1), 
// it ignore copying all deleted tuples
// the function returns the number of tuples copied
uint16_t insert_tuples_from_page(void* page, uint32_t page_size, const tuple_def* def, const void* page_src, uint16_t start_index, uint16_t end_index);

// update tuple at the specified index, fails if the page id out of space, or if the index is out of bounds
// index must be < get_tuple_count()
int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple);

// to delete a tuple at the given index in the page
// delete fails with 0, if index >= get_tuple_count()
// index must be < get_tuple_count()
int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);

// to check if a tuple at the given index in the page exists
// 1 means the tuple exists, else if 0 then the tuple does not exists
// it also returns 0, also when the tuple index is out of bounds, i.e. (index >= get_tuple_count())
// or if it is deleted
int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);




// returns NULL on failure, when the tuple index is out of bounds, i.e. (index >= get_tuple_count())
// returns pointer to nth tuple in the page
const void* get_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index);




// returns 1, if the page_compaction is not a NOOP
// this function is a NOOP for FIXED_ARRAY_PAGE_LAYOUT
int run_page_compaction(void* page, uint32_t page_size, const tuple_def* tpl_d);




// returns total free space inside a given page
// free_space = total_space - space occupied by the tuples (including the deleted one's)
uint32_t get_free_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns the space that is used on this page for storing the tuples between indices start_index and end_index
// this is the space excluding the deleted tuples and the extra left over space between tuples (which is also left over from deleted tuples)
// it is the core data memory used on the page for the tuples, excluding all the unused space
// it also does not include the bitmap size required for storing the tuples (for FIXED_ARRAY_PAGE_LAYOUT)
// essentially this is the ammount of free space required to copy the intended tuples from one page to another, ignoring the deleted ones
uint32_t get_space_occupied_by_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t start_index, uint16_t end_index);




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