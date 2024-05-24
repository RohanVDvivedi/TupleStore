#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>
#include<user_value.h>

// the index must be an unsigned integral value between [0, tpl_d->element_count), for the functions that are called on elements of a tuple for given a tuple definition.
// this is the mandatory condition, since, the functions do not check this condition to be true, repetitively.
// and the functions assume that this condition holds true while you call that particular element level function.

// initialized the tuple with all NULL attributes  (whereever a NULL is possible, this only excludes non-NULLable fixed length elements) and its minimum size
void init_tuple(const tuple_def* tpl_d, void* tupl);

// actual size of the element in the tuple
uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

// get size of tuple, doesn't matter fixed or variable
uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);
uint32_t get_tuple_size_using_tuple_size_def(const tuple_size_def* tpl_sz_d, const void* tupl);

// check if the ith element in the tuple is NULL
int is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

// check if an element can be set (given at index) in tuple at a given index
// returns 1 if the set operation was will succeed,
// else returns 0 (this happens when the types are incompatible OR setting NULL to a non-NULLable element OR the tuple will exceed its max_size)
// On success, (if new_tuple_size != NULL) then new_tupl_size will be set to the new tuple_size that the tuple will achieve
int can_set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const user_value* value, uint32_t* new_tuple_size);

// sets an element (given at index) in tuple at a given index
// returns 1 if the set operation was will succeed,
// else returns 0 (this happens when the types are incompatible OR setting NULL to a non-NULLable element OR the tuple will exceed its max_size)
int set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const user_value* value);

// set element (given at index_in in tuple_in) to tuple at a given index
// returns 1 if the set operation was successfull,
// else returns 0 (this happens when the types are incompatible OR setting NULL to a non-NULLable element OR the tuple will exceed its max_size)
int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const tuple_def* tpl_d_in, uint32_t index_in, const void* tupl_in);

// This functions attempts to check if an element (value) can be set in a tuple (of size prior_tuple_size) with tuple_definition (tpl_d) at the given index
// On success it will return the new_tuple_size of the tuple, that can be used in a subsequent iteration
// This function can be used to calculate the total memeory size required while performing a projection
int can_set_uninitialized_element_in_tuple(const tuple_def* tpl_d, uint32_t index, uint32_t prior_tuple_size, const user_value* value, uint32_t* new_tuple_size);

// does not allocate any memeory
// returns user_value as int_value, uint_value, float_value or double_value for numeral types
// for non numeral types it returns data and data_size, no memory is allocated for non numeral types
// user_value.is_NULL will be set if the element was NULL in tuple
// the return of this function must not be used after the tuple has had any calls to set_element_in_tuple* functions
user_value get_value_from_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

// compare functions for tuple and for an element inside the tuple
int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, uint32_t index1, const void* tup2, const tuple_def* tpl_d2, uint32_t index2);

typedef enum compare_direction compare_direction;
enum compare_direction
{
	INCREASING = 1,
	DECREASING = -1,
};

#define ASC  INCREASING
#define DESC DECREASING

// if element_ids* == NULL, then this functions compares the first element_count number of elements from both the tuples
// if cmp_dir == NULL, then all the elements are compared in INCREASING order, else cmp_dir decides the direction of the comparison
// else it compares the elements in the same order as provided in element_ids* (here element_count denotes size of element_ids* arrays)
int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const uint32_t* element_ids1, const void* tup2, const tuple_def* tpl_d2, const uint32_t* element_ids2, const compare_direction* cmp_dir, uint32_t element_count);


// hash function for tuple and for an element inside the tuple at the specified index
uint64_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint64_t (*hash_func)(const void* data, uint32_t size));

// if element_ids == NULL, then this functions hashes the first element_count number of elements from the tuple
// else it hashes the elements in the same order as provided in element_ids (here element_count denotes its size)
uint64_t hash_tuple(const void* tup, const tuple_def* tpl_d, const uint32_t* element_ids, uint64_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count);


// print function for tuple
void print_tuple(const void* tup, const tuple_def* tpl_d);

#endif

/*
**	Tuple Layout on page
**
**	if all attributes are fixed length
**  for example
**   UINT   -  4 bytes
**   STRING - 16 bytes
**   INT    -  8 bytes
**
**	then all the attributes are stored side by side inline with each other after the "is null" bitmap
**
**	| is_null bitmap - 1 | UINT - 4 |        STRING - 16           |    INT - 8   |
**
**  Since there are only 3 attributes we need only 1 byte to store the is_null bitmap
**  the ith bit of the is_null bitmap tells us that the ith field is null if the bit is set
**
**  similarly, for 17 attribute count we will require a bitmap of size 3 bytes
**
**  If there is atleast a variable sized element in the tuple definition
**  Then the layout of a variable sized tuple definition is used
**
**	In variable sized tuple layout, 
**  The first value that we store is the length of the tuple,
**  then we store is_null bitmap just as in fixed length tuple layout
**  then we store all the attributes inline just as in fixed length tuple layout
**
**  the only difference is that while storing the variable sized elements
**  we only store the the offset of the variable sized element in line with other fixed sized elements
**  the actual data of all variable sized elements are stored at the end of the tuple
**	
**	for example
**   UINT       -  4 bytes
**   VAR_STRING -  (2 bytes to store its length in its prefix)
**   INT        -  8 bytes
**
**  the number of bytes taken by the tuple size and the offsets of the variable sized elements is given by the max tuple size or the size of the page
**  Here we consider that the max tuple size is 4 KB i.e. these fields take up 2 bytes each
**
**	| tuple_size - 2 | is_null bitmap - 1 | UINT - 4 | VAR_STRING - 2 (only offset) |          INT - 8      | Actual VAR_STRING can have any number of bytes |
**
*/