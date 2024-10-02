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
#define NEXT_POSITION(pa) ((positional_accessor){.positions_length = pa.positions_length - 1, .positions = pa.positions + 1}) 	// build a positional accessor for the next nested object
#define STATIC_POSITION(...) ((positional_accessor){ .positions_length = sizeof((uint32_t []){ __VA_ARGS__ })/sizeof(uint32_t), .positions = (uint32_t []){ __VA_ARGS__ } })
// usage STATIC_POSITION(a, b, c, d)

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ structure and macros to index elements inside a tuple nestedly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


// simple accessors

void init_tuple(const tuple_def* tpl_d, void* tupl);

const user_value get_value_from_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

const data_type_info* get_type_info_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

int can_set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed);

uint32_t get_element_count_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

int can_expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots);



// compare and hash accessors

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