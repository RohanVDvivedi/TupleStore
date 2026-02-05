#include<tuplestore/tuple.h>

// (*below macro is for internl use only)
// it initializes a new positional accessor that will make you point to the next position you need to extract next
#define NEXT_POSITION(pa) ((positional_accessor){.positions_length = pa.positions_length - 1, .positions = pa.positions + 1}) 	// build a positional accessor for the next nested object (* for internal use only)

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	initialize_minimal_data_for_type_info(tpl_d->type_info, tupl);
}

int get_value_from_element_from_tuple(user_value* uval, const tuple_def* tpl_d, positional_accessor pa, const void* tupl)
{
	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return get_user_value_for_type_info(uval, dti, data);

			// result is self's some child
			if(pa.positions_length == 1)
			{
				data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
				return get_user_value_to_containee_from_container(uval, dti, data, pa.positions[0], &containee_pos_info);
			}
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		if(child_data == NULL)
		{
			uval->is_NULL = 1;
			return 1;
		}

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

const data_type_info* get_type_info_for_element_from_tuple_def(const tuple_def* tpl_d, positional_accessor pa)
{
	const data_type_info* dti = tpl_d->type_info;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return dti;
		}

		// result is self's some child
		const data_type_info* child_dti = get_data_type_info_for_containee_of_container_without_data(dti, pa.positions[0]);

		if(child_dti == NULL)
			return NULL;

		dti = child_dti;
		pa = NEXT_POSITION(pa);
	}

	return NULL;
}

int are_all_positions_accessible_for_tuple_def(const tuple_def* tpl_d, const positional_accessor* element_ids, uint32_t element_count)
{
	for(uint32_t i = 0; i < element_count; i++)
	{
		const data_type_info* key_dti = get_type_info_for_element_from_tuple_def(tpl_d, ((element_ids == NULL) ? STATIC_POSITION(i) : element_ids[i]));
		if(key_dti == NULL)
			return 0;
	}

	return 1;
}

int are_all_positions_accessible_for_tuple(const void* tupl, const tuple_def* tpl_d, const positional_accessor* element_ids, uint32_t element_count)
{
	for(uint32_t i = 0; i < element_count; i++)
	{
		user_value key;
		if(!get_value_from_element_from_tuple(&key, tpl_d, ((element_ids == NULL) ? STATIC_POSITION(i) : element_ids[i]), tupl))
			return 0;
	}

	return 1;
}

uint32_t get_max_size_increment_allowed_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl)
{
	// start with maximum possible value
	uint32_t max_size_increment_allowed = UINT32_MAX;

	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// if fixed sized dti then return 0, they can not grab more space, even if they want to
		// weeds out bit_fields and primitive numeral type infos and fixed sized containers
		if(!is_variable_sized_type_info(dti))
			return 0;

		// actual compuatation, since this is a variable sized element for sure, due to the check above
		// it's dti has max_size attribute set
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

		// loop termination cases
		{
			// no more nesting, so break out
			if(IS_SELF(pa))
				break;
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		if(child_data == NULL)
			return 0;

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return max_size_increment_allowed;
}

int can_set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, const user_value* value, uint32_t max_size_increment_allowed)
{
	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
			{
				// null is to be set into the container containing this element
				if(is_user_value_NULL(value))
					return 0;

				return can_set_user_value_for_type_info(dti, data, 1, max_size_increment_allowed, value);
			}

			// result is self's some child
			if(pa.positions_length == 1)
			{
				data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
				return can_set_user_value_to_containee_in_container(dti, data, pa.positions[0], max_size_increment_allowed, value, &containee_pos_info);
			}
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		if(is_variable_sized_type_info(dti))
			max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		// if the child nested in middle is null, then we can not set its child
		if(child_data == NULL)
			return 0;

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

static int set_element_in_tuple_INTERNAL(const data_type_info* dti, positional_accessor pa, void* data, const user_value* value, uint32_t max_size_increment_allowed, int is_inner_most_dti_variable_sized)
{
	// recursion termination cases
	{
		// result is self
		if(IS_SELF(pa))
		{
			// null is to be set into the container containing this element
			if(is_user_value_NULL(value))
				return 0;

			return set_user_value_for_type_info(dti, data, 1, max_size_increment_allowed, value);
		}

		// result is self's some child
		if(pa.positions_length == 1)
		{
			data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
			return set_user_value_to_containee_in_container(dti, data, pa.positions[0], max_size_increment_allowed, value, &containee_pos_info);
		}
	}

	data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	if(is_variable_sized_type_info(dti))
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

	// if the inmer_most_dti to be set is variable sized, then move child of current dti to the end
	if(is_inner_most_dti_variable_sized)
		move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

	void* child_data = (void*) get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
	const data_type_info* child_dti = containee_pos_info.type_info;

	// if the child nested in middle is null, then we can not set its child
	if(child_data == NULL)
		return 0;

	int result = 0;
	if(is_inner_most_dti_variable_sized) // if the inner most dti is variable sized, then parent size needs to be updated
	{
		uint32_t container_old_size = get_size_for_type_info(dti, data);
		uint32_t child_old_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

		result = set_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, value, max_size_increment_allowed, is_inner_most_dti_variable_sized);

		if(result)
		{
			uint32_t child_new_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
			overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
		}
	}
	else
	{
		result = set_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, value, max_size_increment_allowed, is_inner_most_dti_variable_sized);
	}

	return result;
}

int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(inner_most_dti == NULL)
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return set_element_in_tuple_INTERNAL(dti, pa, data, value, max_size_increment_allowed, is_variable_sized_type_info(inner_most_dti));
}

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed)
{
	user_value uval_in;
	if(!get_value_from_element_from_tuple(&uval_in, tpl_d_in, pa_in, tupl_in))
		return 0;
	const data_type_info* dti_in = get_type_info_for_element_from_tuple_def(tpl_d_in, pa_in);
	if(dti_in == NULL)
		return 0;
	const data_type_info* dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(dti == NULL)
		return 0;

	if(are_identical_type_info(dti, dti_in)) // if both are logically identical types, no type casting required
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in, max_size_increment_allowed);
	else if(is_primitive_numeral_type_info(dti) && is_primitive_numeral_type_info(dti_in)) // primitive numeral type info's are aften internally type castable
	{
		user_value uval_in_settable = {};
		if(!type_cast_primitive_numeral_type(&uval_in_settable, dti, &uval_in, dti_in))
			return 0;
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in_settable, max_size_increment_allowed);
	}
	else if((dti->type == STRING || dti->type == BINARY) && (dti_in->type == STRING || dti_in->type == BINARY)) // STRING and BINARY values of different sized are internally type castable
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in, max_size_increment_allowed); // string_value, binary_value and string_or_binary_value are interchangeable
	else
		return 0;
}

uint32_t get_element_count_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl)
{
	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return get_element_count_for_container_type_info(dti, data);
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		// if the child nested in middle is null, then we can not get element count of its child
		if(child_data == NULL)
			return 0;

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

int can_expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(inner_most_dti == NULL || !is_variable_element_count_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return can_expand_container(dti, data, index, slots, max_size_increment_allowed);
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// inner_most_dti is variable element_count, so it is bydefault a variable sized data type
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		// if the child nested in middle is null, then we can not expand its child
		if(child_data == NULL)
			return 0;

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

static int expand_element_count_for_element_in_tuple_INTERNAL(const data_type_info* dti, positional_accessor pa, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// loop termination cases
	{
		// result is self
		if(IS_SELF(pa))
			return expand_container(dti, data, index, slots, max_size_increment_allowed);
	}

	data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// inner_most_dti is variable element_count, so it is bydefault a variable sized data type
	max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

	// then move child of current dti to the end
	move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

	void* child_data = (void*) get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
	const data_type_info* child_dti = containee_pos_info.type_info;

	// if the child nested in middle is null, then we can not expand its child
	if(child_data == NULL)
		return 0;

	uint32_t container_old_size = get_size_for_type_info(dti, data);
	uint32_t child_old_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

	int result = expand_element_count_for_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, index, slots, max_size_increment_allowed);

	if(result)
	{
		uint32_t child_new_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
	}

	return result;
}

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(inner_most_dti == NULL || !is_variable_element_count_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return expand_element_count_for_element_in_tuple_INTERNAL(dti, pa, data, index, slots, max_size_increment_allowed);
}

int can_discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(inner_most_dti == NULL || !is_variable_element_count_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return can_discard_from_container(dti, data, index, slots);
		}

		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// inner_most_dti is variable element_count, so it is by default a variable sized data type

		const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		const data_type_info* child_dti = containee_pos_info.type_info;

		// if the child nested in middle is null, then we can not discard from its child
		if(child_data == NULL)
			return 0;

		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

static int discard_elements_from_element_in_tuple_INTERNAL(const data_type_info* dti, positional_accessor pa, void* data, uint32_t index, uint32_t slots)
{
	// loop termination cases
	{
		// result is self
		if(IS_SELF(pa))
			return discard_from_container(dti, data, index, slots);
	}

	data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;

	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// inner_most_dti is variable element_count, so it is by default a variable sized data type

	// then move child of current dti to the end
	move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

	void* child_data = (void*) get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
	const data_type_info* child_dti = containee_pos_info.type_info;

	// if the child nested in middle is null, then we can not discard from its child
	if(child_data == NULL)
		return 0;

	uint32_t container_old_size = get_size_for_type_info(dti, data);
	uint32_t child_old_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);

	int result = discard_elements_from_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, index, slots);

	if(result)
	{
		uint32_t child_new_size = get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, pa.positions[0], &containee_pos_info);
		overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
	}

	return result;
}

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(inner_most_dti == NULL || !is_variable_element_count_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return discard_elements_from_element_in_tuple_INTERNAL(dti, pa, data, index, slots);
}

// compare and hash functions
int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const void* tup2, const tuple_def* tpl_d2, positional_accessor pa2)
{
	// if the element is not accessible, then fail
	const data_type_info* dti1 = get_type_info_for_element_from_tuple_def(tpl_d1, pa1);
	if(dti1 == NULL)
		return -2;

	// if the element is not accessible, then fail
	const data_type_info* dti2 = get_type_info_for_element_from_tuple_def(tpl_d2, pa2);
	if(dti2 == NULL)
		return -2;

	// get the user value for this element
	user_value uval1;
	if(!get_value_from_element_from_tuple(&uval1, tpl_d1, pa1, tup1))
		return -2;

	// get the user value for this element
	user_value uval2;
	if(!get_value_from_element_from_tuple(&uval2, tpl_d2, pa2, tup2))
		return -2;

	// TODO : handle logic for custom compare function

	if(dti1 == dti2) // there is slight possibility of this to be true, in case of comparision between key entry, index entry and record entry of a bplus tree index
		return compare_user_value2(&uval1, &uval2, dti2);
	else
		return compare_user_value(&uval1, dti1, &uval2, dti2);
}

int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const positional_accessor* element_ids1, const void* tup2, const tuple_def* tpl_d2, const positional_accessor* element_ids2, const compare_direction* cmp_dir, uint32_t element_count)
{
	// edge case when comparing same types
	if(tpl_d1 == tpl_d2 && element_ids1 == element_ids2)
		return compare_tuples2(tup1, tup2, tpl_d2, element_ids2, cmp_dir, element_count);

	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
	{
		compare = compare_elements_of_tuple(tup1, tpl_d1, ((element_ids1 == NULL) ? STATIC_POSITION(i) : element_ids1[i]), tup2, tpl_d2, ((element_ids2 == NULL) ? STATIC_POSITION(i) : element_ids2[i]));

		if(compare == -2) // this implies elements are not comparable
			return -2;

		// if cmp_dir is not NULL, then compare in default direction of the element
		if(cmp_dir != NULL)
			compare = compare * cmp_dir[i];
	}
	return compare;
}

int compare_element_with_user_value(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const user_value* uval2, const data_type_info* dti2)
{
	// if the element is not accessible, then fail
	const data_type_info* dti1 = get_type_info_for_element_from_tuple_def(tpl_d1, pa1);
	if(dti1 == NULL)
		return -2;

	// get the user value for this element
	user_value uval1;
	if(!get_value_from_element_from_tuple(&uval1, tpl_d1, pa1, tup1))
		return -2;

	// TODO : handle logic for custom compare function

	if(dti1 == dti2) // there is slight possibility of this to be true, in case of comparision between key entry, index entry and record entry of a bplus tree index
		return compare_user_value2(&uval1, uval2, dti2);
	else
		return compare_user_value(&uval1, dti1, uval2, dti2);
}

int compare_tuple_with_user_value(const void* tup1, const tuple_def* tpl_d1, const positional_accessor* element_ids1, const user_value* uvals2, data_type_info const * const * dtis2, const compare_direction* cmp_dir, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
	{
		compare = compare_element_with_user_value(tup1, tpl_d1, ((element_ids1 == NULL) ? STATIC_POSITION(i) : element_ids1[i]), uvals2 + i, dtis2[i]);

		if(compare == -2) // this implies elements are not comparable
			return -2;

		// if cmp_dir is not NULL, then compare in default direction of the element
		if(cmp_dir != NULL)
			compare = compare * cmp_dir[i];
	}
	return compare;
}

int compare_elements_of_tuple2(const void* tup1, const void* tup2, const tuple_def* tpl_d, positional_accessor pa)
{
	// if the element is not accessible, then fail
	const data_type_info* dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(dti == NULL)
		return -2;

	// get the user value for this element
	user_value uval1;
	if(!get_value_from_element_from_tuple(&uval1, tpl_d, pa, tup1))
		return -2;

	// get the user value for this element
	user_value uval2;
	if(!get_value_from_element_from_tuple(&uval2, tpl_d, pa, tup2))
		return -2;

	// TODO : handle logic for custom compare function

	return compare_user_value2(&uval1, &uval2, dti);
}

int compare_tuples2(const void* tup1, const void* tup2, const tuple_def* tpl_d, const positional_accessor* element_ids, const compare_direction* cmp_dir, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
	{
		compare = compare_elements_of_tuple2(tup1, tup2, tpl_d, ((element_ids == NULL) ? STATIC_POSITION(i) : element_ids[i]));

		if(compare == -2) // this implies elements are not comparable
			return -2;

		// if cmp_dir is not NULL, then compare in default direction of the element
		if(cmp_dir != NULL)
			compare = compare * cmp_dir[i];
	}
	return compare;
}

int compare_user_values3(const user_value* uvals1, const user_value* uvals2, data_type_info const * const * dtis, const compare_direction* cmp_dir, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
	{
		compare = compare_user_value2(uvals1 + i, uvals2 + i, dtis[i]);

		if(compare == -2) // this implies elements are not comparable
			return -2;

		// if cmp_dir is not NULL, then compare in default direction of the element
		if(cmp_dir != NULL)
			compare = compare * cmp_dir[i];
	}
	return compare;
}

uint64_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, positional_accessor pa, tuple_hasher* th)
{
	// if the element is not accessible, then fail
	const data_type_info* dti = get_type_info_for_element_from_tuple_def(tpl_d, pa);
	if(dti == NULL)
		return th->hash;

	// get the user value for this element
	user_value uval;
	if(!get_value_from_element_from_tuple(&uval, tpl_d, pa, tup))
		return th->hash;

	return hash_user_value(&uval, dti, th);
}

uint64_t hash_tuple(const void* tup, const tuple_def* tpl_d, const positional_accessor* element_ids, tuple_hasher* th, uint32_t element_count)
{
	if(element_ids == NULL)
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_element_within_tuple(tup, tpl_d, STATIC_POSITION(i), th);
	}
	else
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_element_within_tuple(tup, tpl_d, element_ids[i], th);
	}
	return th->hash;
}

// print function

void print_tuple(const void* tup, const tuple_def* tpl_d)
{
	print_data_for_data_type_info(tpl_d->type_info, tup);
	printf("\n");
}