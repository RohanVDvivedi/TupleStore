#include<tuple.h>

#include<stdio.h>
#include<inttypes.h>
#include<alloca.h>
#include<string.h>

#include<bitmap.h>

#include<tuple_def.h>
#include<int_accesses.h>
#include<numeral_element_types.h>
#include<string_or_blob_element_types_util.h>
#include<page_layout_util.h>

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	// set all the is_NULL_bitmap bits to 1
	set_all_bits(tupl + tpl_d->byte_offset_to_is_null_bitmap, tpl_d->is_NULL_bitmap_size_in_bits);

	// set its size to minimum size of the tuple
	if(is_variable_sized_tuple_def(tpl_d))
		write_uint32(tupl, tpl_d->size_def.size_of_byte_offsets, get_minimum_tuple_size(tpl_d));

	// set all offsets to variable sized elements to 0
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tpl_d); i++)
	{
		const element_def* ele_d = get_element_def_by_id(tpl_d, i);
		if(is_variable_sized_element_def(ele_d))
			write_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets, 0);
	}
}

// do not use this function directly, call get_element_from_tuple macro
static uint32_t get_element_offset_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);
	if(is_fixed_sized_element_def(ele_d)) // i.e. fixed sized
		return ele_d->byte_offset;
	else
		return read_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets);
}

#define get_element_from_tuple(tpl_d, index, tupl) (is_NULL_in_tuple(tpl_d, index, tupl) ? NULL : ((tupl) + get_element_offset_within_tuple(tpl_d, index, tupl)))

uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	// for a NULL "variable sized element", no space is allocated for its actual contents (there is only space for its offset (from the first byte of the tuple))
	if(is_variable_sized_element_def(ele_d) && is_NULL_in_tuple(tpl_d, index, tupl))
		return 0;
	else
	{
		const void* e = get_element_from_tuple(tpl_d, index, tupl);
		return get_element_size(e, ele_d);
	}
}

uint32_t get_tuple_size_using_tuple_size_def(const tuple_size_def* tpl_sz_d, const void* tupl)
{
	if(is_fixed_sized_tuple_size_def(tpl_sz_d)) // i.e. fixed sized tuple
		return tpl_sz_d->size;
	else // for a variable sized tuple the first few bytes are used to store its size
	{
		uint32_t tuple_size = read_uint32(tupl, tpl_sz_d->size_of_byte_offsets);

		// a tuple_size of 0 implies a tuple_size of max_size, A tuple_size of 0 is invalid
		if(tuple_size == 0)
			tuple_size = tpl_sz_d->max_size;

		return tuple_size;
	}
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	return get_tuple_size_using_tuple_size_def(&(tpl_d->size_def), tupl);
}

int is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	// element_def in concern
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	// check the is_NULL bit for fixed sized element def
	if(has_bit_in_is_NULL_bitmap(ele_d))
		return get_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, ele_d->is_NULL_bitmap_bit_offset);
	else if(is_variable_sized_element_def(ele_d))// else for a variable sized element, check its offset, if the offset is 0, then the element is NULL
		return (0 == read_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets));
	else // else it is NOT null or just can not be NULL
		return 0;
}

// this function only sets an element to NULL, if possible, it is a utility function
// public api must call only the set_element_* functions, this function will be used by the set_element_* function internally
static int set_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl)
{
	// element_def in concern
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	int done = 0;

	// set its is_NULL bit if it has 1
	if(has_bit_in_is_NULL_bitmap(ele_d))
	{
		set_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, ele_d->is_NULL_bitmap_bit_offset);
		done = 1;
	}

	// for a variable sized element set its offset to 0
	if(is_variable_sized_element_def(ele_d))
	{
		write_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets, 0);
		done = 1;
	}

	return done;
}

static int reset_NULL_bit_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl)
{
	// element_def in concern
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	// reset is_NULL bit if it has one
	if(has_bit_in_is_NULL_bitmap(ele_d))
	{
		reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, ele_d->is_NULL_bitmap_bit_offset);
		return 1;
	}

	return 0;
}

int can_set_element_in_tuple_from_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const user_value* value, uint32_t* new_tuple_size)
{
	// element definition we are concerned with
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	// can not set NULL to a non-NULLable element def
	if(is_user_value_NULL(value) && !is_NULLable_element_def(ele_d))
		return 0;

	uint32_t original_tuple_size = get_tuple_size(tpl_d, tupl);

	if(is_fixed_sized_element_def(ele_d)) // if the updated element is a fixed_sized element, then the tuple already has space for the new element, no extra space required
	{
		if(new_tuple_size)
			(*new_tuple_size) = original_tuple_size;
		return 1;
	}
	// when a variable sized element is updated, it changes the tuple_size
	// we must ensure that it does not cross the max_size stated in the tpl_d->size_def.max_size
	else
	{
		// old element needs to be removed
		uint32_t final_tuple_size = original_tuple_size - get_element_size_within_tuple(tpl_d, index, tupl);

		// compute new_element_size, that is to be inserted
		uint32_t new_element_size = 0;
		if(!is_user_value_NULL(value))
		{
			if(will_unsigned_sum_overflow(uint32_t, ele_d->size_specifier_prefix_size, value->data_size))
				return 0;
			new_element_size = ele_d->size_specifier_prefix_size + value->data_size;
		}

		if(will_unsigned_sum_overflow(uint32_t, final_tuple_size, new_element_size) || (final_tuple_size + new_element_size) > get_maximum_tuple_size(tpl_d))
			return 0;

		// we can now add the new_element_size to the final_tuple_size
		final_tuple_size += new_element_size;

		if(new_tuple_size)
			(*new_tuple_size) = final_tuple_size;
		return 1;
	}
}

int set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const user_value* value)
{
	// check if an element can be set in tuple
	if(!can_set_element_in_tuple_from_tuple(tpl_d, index, tupl, value, NULL))
		return 0;

	// element definition we are concerned with
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);

	// if the user has requested to set the value to default value
	// then set the value to the specific default value provided by the user in the element def
	if(value == DEFAULT_USER_VALUE)
		value = &(ele_d->default_value);

	// if the element inside tuple is NULL, and we are asked to set it to NULL, then return
	if(is_NULL_in_tuple(tpl_d, index, tupl) && is_user_value_NULL(value))
		return 1;

	if(is_fixed_sized_element_def(ele_d))
	{
		// if the value to be set is NULL
		if(is_user_value_NULL(value))
		{
			// here we are sure that the element in the tuple is not NULL

			// reset the corresponding bytes (of the fixed length element)
			void* ele = get_element_from_tuple(tpl_d, index, tupl);
			uint32_t ele_size = get_element_size_within_tuple(tpl_d, index, tupl);
			memset(ele, 0, ele_size);

			// then just set the corresponding bit in the is_null bitmap
			set_NULL_in_tuple(tpl_d, index, tupl);
		}
		else
		{
			// set the is_null bitmap bit to 0
			reset_NULL_bit_in_tuple(tpl_d, index, tupl);

			// this won't return a NULL element because, we just resetted this element's is_null_bitmap bit
			void* ele = get_element_from_tuple(tpl_d, index, tupl);

			if(is_numeral_type_element_def(ele_d))
				set_numeral_element(ele, ele_d, value);
			else
				set_string_OR_blob_element(ele, ele_d, value);
		}
	}
	else
	{
		// if data existed at index (is not NULL), then remove it (its allocated space), set its offset to 0 and set it's is_null_bitmap bit to 1
		if(!is_NULL_in_tuple(tpl_d, index, tupl))
		{
			uint32_t old_element_size = get_element_size_within_tuple(tpl_d, index, tupl);	// this will not be 0 because the element is not NULL
			uint32_t old_element_offset = get_element_offset_within_tuple(tpl_d, index, tupl);

			uint32_t old_tuple_size = get_tuple_size(tpl_d, tupl);

			// move all the bytes in the tuple after this element to front
			memmove(tupl + old_element_offset, tupl + old_element_offset + old_element_size, old_tuple_size - (old_element_offset + old_element_size));

			// decrease all tuple offsets for variable sized non null elements that were after the element by old_element_size
			for(uint32_t i = 0; i < get_element_def_count_tuple_def(tpl_d); i++)
			{
				if(!is_NULL_in_tuple(tpl_d, index, tupl) && is_variable_sized_element_def(get_element_def_by_id(tpl_d, i)))
				{
					uint32_t offset = get_element_offset_within_tuple(tpl_d, i, tupl);
					if(offset > old_element_offset)
						write_uint32(tupl + get_element_def_by_id(tpl_d, i)->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets, offset - old_element_size);
				}
			}

			// update tuple size to old_tuple_size - old_element_size
			write_uint32(tupl, tpl_d->size_def.size_of_byte_offsets, old_tuple_size - old_element_size);
			
			// set the offset of that element to 0, making it NULL
			set_NULL_in_tuple(tpl_d, index, tupl);
		}

		// at this point this element is NULL and has no space allocated for it on the tuple

		// now if there is a value to set
		// then reset its is_null_bitmap bit, allocate space for this new (variable sized) element, and set the new data with value
		if(!is_user_value_NULL(value))
		{
			// get current tuple_size
			uint32_t tuple_size = get_tuple_size(tpl_d, tupl);

			// allocate space at the end of the tuple for this element
			// its new offset will be tuple_size
			// update its offset on the tuple
			// this will also make this element a non NULL
			write_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets, tuple_size);

			// since the offset is set appropriately
			// we can access element directly and safely
			void* ele = get_element_from_tuple(tpl_d, index, tupl);

			// set the string or blob element
			set_string_OR_blob_element(ele, ele_d, value);

			// update tuple size to tuple_size
			tuple_size += get_element_size(ele, ele_d);
			write_uint32(tupl, tpl_d->size_def.size_of_byte_offsets, tuple_size);
		}
	}

	return 1;
}

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const tuple_def* tpl_d_in, uint32_t index_in, const void* tupl_in)
{
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);
	const element_def* ele_d_in = get_element_def_by_id(tpl_d_in, index_in);

	// if the elements can not be set from one another, then return 0 (failure)
	if(!can_set_from_element_defs(ele_d, ele_d_in))
		return 0;

	// attmepting to set a NULL to a non-NULLable field must fail
	if(is_NULL_in_tuple(tpl_d_in, index_in, tupl_in) && !is_NULLable_element_def(ele_d))
		return 0;

	// if the index_in-th element in the tuple is NULL then set index-th element in tuple as NULL
	if(is_NULL_in_tuple(tpl_d_in, index_in, tupl_in))
		return set_element_in_tuple(tpl_d, index, tupl, NULL);

	// For numeric types
	if(is_numeral_type_element_def(ele_d) && is_numeral_type_element_def(ele_d_in))
	{
		// set the is_null bitmap bit to 0, of the corresponding element in tupl at the given index
		reset_NULL_bit_in_tuple(tpl_d, index, tupl);
		
		const void* ele_in = get_element_from_tuple(tpl_d_in, index_in, tupl_in);
		void* ele = get_element_from_tuple(tpl_d, index, tupl);

		// this function takes care of typecasting (when writing from lets sat FLOAT to UINT, etc)
		set_numeral_element_from_element(ele, ele_d, ele_in, ele_d_in);

		return 1;
	}

	// if both of them are string or blob like types then
	else
	{
		// get a user value corresponding to the *_in element
		user_value value_in = get_value_from_element_from_tuple(tpl_d_in, index_in, tupl_in);

		// and now set this user value in the tuple
		return set_element_in_tuple(tpl_d, index, tupl, &value_in);
	}
}

user_value get_value_from_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	const element_def* ele_d = get_element_def_by_id(tpl_d, index);
	const void* e = get_element_from_tuple(tpl_d, index, tupl);

	if(e == NULL)
		return (*NULL_USER_VALUE);

	if(is_numeral_type_element_def(ele_d))
		return get_value_from_numeral_element(e, ele_d);
	else
		return get_value_from_string_OR_blob_element(e, ele_d);
}

int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, uint32_t index1, const void* tup2, const tuple_def* tpl_d2, uint32_t index2)
{
	const element_def* ele_d1 = get_element_def_by_id(tpl_d1, index1);
	const element_def* ele_d2 = get_element_def_by_id(tpl_d2, index2);

	if(!can_compare_element_defs(ele_d1, ele_d2))
		return -2;

	const void* e1 = get_element_from_tuple(tpl_d1, index1, tup1);
	const void* e2 = get_element_from_tuple(tpl_d2, index2, tup2);

	// handling case when elements are NULL
	if(e1 == NULL && e2 == NULL)
		return 0;
	else if(e1 == NULL || e2 == NULL)
	{
		// a NULL element is always considered lesser than a NON NULL element
		if(e1 == NULL)
			return -1;
		else
			return 1;
	}
	else
		return compare_elements(e1, get_element_def_by_id(tpl_d1, index1), e2, get_element_def_by_id(tpl_d2, index2));
}

int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const uint32_t* element_ids1, const void* tup2, const tuple_def* tpl_d2, const uint32_t* element_ids2, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
		compare = compare_elements_of_tuple(tup1, tpl_d1, ((element_ids1 == NULL) ? i : element_ids1[i]), tup2, tpl_d2, ((element_ids2 == NULL) ? i : element_ids2[i]));
	return compare;
}

uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	const void* e = get_element_from_tuple(tpl_d, index, tup);

	// hashing a NULL element returns a 0
	if(e == NULL)
		return 0;

	const element_def* ele_d = get_element_def_by_id(tpl_d, index);
	return hash_element(e, ele_d, hash_func);
}

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count, const uint32_t* element_ids)
{
	uint32_t hash_value = 0;
	if(element_ids == NULL)
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, i, hash_func);
	}
	else
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, element_ids[i], hash_func);
	}
	return hash_value;
}

void print_tuple(const void* tup, const tuple_def* tpl_d)
{
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tpl_d); i++)
	{
		if(i)
			printf(", ");

		user_value ele_val = get_value_from_element_from_tuple(tpl_d, i, tup);

		if(is_user_value_NULL(&ele_val))
		{
			printf("NULL");
			continue;
		}

		const element_def* ele_d = get_element_def_by_id(tpl_d, i);

		if(is_variable_sized_element_def(ele_d))
			printf("[%"PRIu32"]->", read_uint32(tup + ele_d->byte_offset_to_byte_offset, tpl_d->size_def.size_of_byte_offsets));

		print_user_value(&ele_val, ele_d);
	}
	printf("\n");
}
