#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

// ------------------------------ structure and macros to index elements inside a tuple nestedly ----------------------------------------

// used to access conatiner data type info's nestedly like TUPLE, ARRAY, STRING and BLOB only
// stores the complete path to access the object
typedef struct positional_accessor positional_accessor;
struct positional_accessor
{
	uint32_t positions_length;

	uint32_t* positions;
};

// for a given positional accessor the nested child to work on is at index positions[0], if the positions_length > 0

// utiities for positional accessors
#define SELF ((positional_accessor){.positions_length = 0, .positions = NULL})													// point to self
#define IS_SELF(pa) (pa.positions_length == 0)																					// check if points to self
#define STATIC_POSITION(...) ((positional_accessor){ .positions_length = sizeof((uint32_t []){ __VA_ARGS__ })/sizeof(uint32_t), .positions = (uint32_t []){ __VA_ARGS__ } })
// usage STATIC_POSITION(a, b, c, d)

// below is a utility function to append t to pa, this allows someone to access a nexted element in pa, with a relative index of t
// this function assumes that pa->positions is large enough to accomodate relative position in t
static inline void append_positions(positional_accessor* pa, positional_accessor t)
{
	memory_move(pa->positions + pa->positions_length, t.positions, t.positions_length * sizeof(uint32_t));
	pa->positions_length += t.positions_length;
}

static inline int point_to_parent_position(positional_accessor* pa) // pops child most element from the pa
{
	if(IS_SELF((*pa)))
		return 0;
	pa->positions_length--;
	return 1;
}

static inline int point_to_next_sibling_position(positional_accessor* pa) // increments the last position
{
	if(IS_SELF((*pa)))
		return 0;
	pa->positions[pa->positions_length-1]++;
	return 1;
}

static inline void point_to_i_th_child_position(positional_accessor* pa, uint32_t i) // pushes i onto the pa
{
	append_positions(pa, STATIC_POSITION(i));
}

static inline void point_to_first_child_position(positional_accessor* pa) // point_to_i_th_child_position(0)
{
	point_to_i_th_child_position(pa, 0);
}

static inline int point_to_uncle_position(positional_accessor* pa) // point_to_parent_position + point_to_next_sibling_position
{
	return point_to_parent_position(pa) && point_to_next_sibling_position(pa);
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ structure and macros to index elements inside a tuple nestedly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


// simple accessors

void init_tuple(const tuple_def* tpl_d, void* tupl);

int get_value_from_element_from_tuple(user_value* uval, const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

const data_type_info* get_type_info_for_element_from_tuple_def(const tuple_def* tpl_d, positional_accessor pa);

// below 2 functions can be used to ensure that no positions are OUT_OF_BOUNDS or inaccessible for the given tple and tuple def
// these functions should be used prior to accessing key elements in the record
int are_all_positions_accessible_for_tuple_def(const tuple_def* tpl_d, const positional_accessor* element_ids, uint32_t element_count);
int are_all_positions_accessible_for_tuple(const void* tupl, const tuple_def* tpl_d, const positional_accessor* element_ids, uint32_t element_count);

// get maximum size increment that is allowable for an element in the tuple
// returns 0 for fixed_sized elements, NULL elements and out of bounds indexes
uint32_t get_max_size_increment_allowed_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

// tupl must be initialized using init_tuple, before you call this function
int can_set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

// tupl must be initialized using init_tuple, before you call this function
int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

// tupl must be initialized using init_tuple, before you call this function
// does type casting internally for primitive numeral types, if possible then settable
// strings and blobs are also internally type castable, and hence settable
// if they are tuples their dti-s must match to be settable
// arrays are not assignable using this function
// while workin with STRING, BLOB, TUPLE and ARRAY types you must ensure that you are workin with 2 separate tuples, because their user_values point to data in tupl_in
int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed);

uint32_t get_element_count_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

int can_expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int can_discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots);

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots);

/*
	For all the above functions that have a can_MODIFY() for its corresponding MODIFY() call
	then you must first ensure that the modification is possible, using the can_MODIFY() function, and then call MODIFY() function
	because the MODIFY() call may end up modifying the tuple, yet fail to modify, this is because we can not know inadvance if the modification would succeed to a nested element
*/


// compare and hash accessors

// compare functions for tuple and for an element inside the tuple
// returns -2 if they are not comparable OR if one of the element is out if bounds
int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const void* tup2, const tuple_def* tpl_d2, positional_accessor pa2);

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
// returns -2 if they are not comparable OR if one of the element is out if bounds
int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const positional_accessor* element_ids1, const void* tup2, const tuple_def* tpl_d2, const positional_accessor* element_ids2, const compare_direction* cmp_dir, uint32_t element_count);

int compare_element_with_user_value(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const user_value* uval2, const data_type_info* dti2);

int compare_tuple_with_user_value(const void* tup1, const tuple_def* tpl_d1, const positional_accessor* element_ids1, const user_value* uvals2, data_type_info const * const * dtis2, const compare_direction* cmp_dir, uint32_t element_count);

int compare_elements_of_tuple2(const void* tup1, const void* tup2, const tuple_def* tpl_d, positional_accessor pa);

int compare_tuples2(const void* tup1, const void* tup2, const tuple_def* tpl_d, const positional_accessor* element_ids, const compare_direction* cmp_dir, uint32_t element_count);

// function added just to support external merge sort
int compare_user_values3(const user_value* uvals1, const user_value* uvals2, data_type_info const * const * dtis, const compare_direction* cmp_dir, uint32_t element_count);

#include<tuple_hasher.h>

// hash function for tuple and for an element inside the tuple at the specified position pa
uint64_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, positional_accessor pa, tuple_hasher* th);

// if element_ids == NULL, then this functions hashes the first element_count number of elements from the tuple
// else it hashes the elements in the same order as provided in element_ids (here element_count denotes its size)
uint64_t hash_tuple(const void* tup, const tuple_def* tpl_d, const positional_accessor* element_ids, tuple_hasher* th, uint32_t element_count);



// print function for tuple
void print_tuple(const void* tup, const tuple_def* tpl_d);

#endif