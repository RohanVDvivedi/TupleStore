#ifndef POSITIONAL_ACCESSOR_H
#define POSITIONAL_ACCESSOR_H

// ------------------------------ structure and macros to index elements inside a tuple nestedly ----------------------------------------

// used to access conatiner data type info's nestedly like TUPLE, ARRAY, STRING and BINARY only
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

static inline int point_to_next_uncle_position(positional_accessor* pa) // logically => point_to_parent_position + point_to_next_sibling_position
{
	if(pa->positions_length <= 1) // SELF and SELF's children do not have uncles
		return 0;
	pa->positions_length--; // point_to_parent_position
	pa->positions[pa->positions_length-1]++; // point_to_next_sibling_position
	return 1;
}

/*
	pseudocode to iterate over all elements or types, from the base position, in pre-order traversal, using the above positional accessor functions

	void your_function(const tuple_def* tpl_d, const void* tupl, const positional_accessor base_position, uint32_t max_relative_depth)
	{
		uint32_t max_absolute_depth = base_position.positions_length + max_relative_depth;
		positional_accessor absolute_position = {.positions = malloc(sizeof(uint32_t) * max_absolute_depth), .positions_length = 0};

		append_positions(&absolute_position, base_position);

		while(1)
		{
			datum uval;
			int valid = get_value_from_element_from_tuple(&uval, tpl_d, absolute_position, tupl);
			const data_type_info* dti = get_type_info_for_element_from_tuple_def(tpl_d, absolute_position);

			if((!valid) || (if you are iterating over types and the parent of dti is an array/string/binary and the absolute_position is at the child == 1) )
			{
				if((absolute_position.positions_length >= base_position.positions_length + 2) && point_to_next_uncle_position(&absolute_position))
					continue;
				else
					break;
			}

			// set any one of the following after analyzing the current element/type
			int found_result = 0;
			int skip_all_remaining_siblings = 0;
			int skip_all_children = 0;

			// analyze dti and datum

			if(found_result) // you found what you wanted, then just break out
				break;
			else if(skip_all_remaining_siblings) // just processed the candidate and want to skip all its siblings
			{
				if((absolute_position.positions_length >= base_position.positions_length + 2) && point_to_next_uncle_position(&absolute_position))
					continue;
				else
					break;
			}
			else if(skip_all_children) // just processed the candidate and want to skip all its children
			{
				if(absolute_position.positions_length > base_position.positions_length)
				{
					point_to_next_sibling_position(&absolute_position);
					continue;
				}
				else
					break;
			}

			// default way to go next
			if((absolute_position.positions_length < max_absolute_depth) && is_container_type_info(dti) && !is_datum_NULL(&uval))
			{
				point_to_first_child_position(&absolute_position);
				continue;
			}
			else
			{
				if(absolute_position.positions_length > base_position.positions_length)
				{
					point_to_next_sibling_position(&absolute_position);
					continue;
				}
				else
					break;
			}
		}

		free(absolute_position.positions);
	}
*/

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ structure and macros to index elements inside a tuple nestedly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif