#include<tuple.h>

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	initialize_minimal_data_for_type_info(tpl_d->type_info, tupl);
}

const user_value get_value_from_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl)
{
	const data_type_info* dti = tpl_d->type_info;
	const void* data = tupl;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return get_user_value_for_type_info(dti, data);

			// result is self's some child
			if(pa.positions_length == 1)
				return get_user_value_to_containee_from_container(dti, data, pa.positions[0]);
		}

		if(!is_container_type_info(dti))
			return *OUT_OF_BOUNDS_USER_VALUE;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return *OUT_OF_BOUNDS_USER_VALUE;

		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return *NULL_USER_VALUE;

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return *NULL_USER_VALUE;
}

const data_type_info* get_type_info_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa)
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
		const data_type_info* key_dti = get_type_info_for_element_from_tuple(tpl_d, ((element_ids == NULL) ? STATIC_POSITION(i) : element_ids[i]));
		if(key_dti == NULL)
			return 0;
	}

	return 1;
}

int are_all_positions_accessible_for_tuple(const void* tupl, const tuple_def* tpl_d, const positional_accessor* element_ids, uint32_t element_count)
{
	for(uint32_t i = 0; i < element_count; i++)
	{
		const user_value key = get_value_from_element_from_tuple(tpl_d, ((element_ids == NULL) ? STATIC_POSITION(i) : element_ids[i]), tupl);
		if(is_user_value_OUT_OF_BOUNDS(&key))
			return 0;
	}

	return 1;
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
				return can_set_user_value_to_containee_in_container(dti, data, pa.positions[0], max_size_increment_allowed, value);
		}

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// if the child nested in middle is null, then we can not set its child
		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return 0;

		if(is_variable_sized_type_info(dti))
			max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
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
			return set_user_value_to_containee_in_container(dti, data, pa.positions[0], max_size_increment_allowed, value);
	}

	
	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if the child nested in middle is null, then we can not set its child
	if(is_containee_null_in_container(dti, data, pa.positions[0]))
		return 0;

	if(is_variable_sized_type_info(dti))
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

	// if the inmer_most_dti to be set is variable sized, then move child of current dti to the end
	if(is_inner_most_dti_variable_sized)
		move_variable_sized_containee_to_end_of_container(dti, data, pa.positions[0]);

	const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
	void* child_data = (void*) get_pointer_to_containee_from_container(dti, data, pa.positions[0]);

	uint32_t container_old_size = get_size_for_type_info(dti, data);
	uint32_t child_old_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);

	int result = set_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, value, max_size_increment_allowed, is_inner_most_dti_variable_sized);

	if(result && is_inner_most_dti_variable_sized)
	{
		uint32_t child_new_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);
		overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
	}

	return result;
}

int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed)
{
	// can not set using an OUT_OF_BOUNDS user value
	if(is_user_value_OUT_OF_BOUNDS(value))
		return 0;

	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL)
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return set_element_in_tuple_INTERNAL(dti, pa, data, value, max_size_increment_allowed, is_variable_sized_type_info(inner_most_dti));
}

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed)
{
	const user_value uval_in = get_value_from_element_from_tuple(tpl_d_in, pa_in, tupl_in);
	if(is_user_value_OUT_OF_BOUNDS(&uval_in))
		return 0;
	const data_type_info* dti_in = get_type_info_for_element_from_tuple(tpl_d_in, pa_in);
	if(dti_in == NULL)
		return 0;
	const data_type_info* dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(dti == NULL)
		return 0;

	if(is_primitive_numeral_type_info(dti))
	{
		if(!is_primitive_numeral_type_info(dti_in))
			return 0;
		if(!can_type_cast_primitive_numeral_type(dti, dti_in))
			return 0;
		user_value uval_in_settable = {};
		type_cast_primitive_numeral_type(&uval_in_settable, dti, &uval_in, dti_in);
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in_settable, max_size_increment_allowed);
	}
	else if((dti->type == STRING || dti->type == BLOB) && (dti_in->type == STRING || dti_in->type == BLOB))
	{
		user_value uval_in_settable;
		if(dti->type == BLOB && dti_in->type == STRING)
			uval_in_settable = (user_value){.blob_value = uval_in.string_value, .blob_size = uval_in.string_size};
		else if(dti->type == STRING && dti_in->type == BLOB)
			uval_in_settable = (user_value){.string_value = uval_in.blob_value, .string_size = uval_in.blob_size};
		else
			uval_in_settable = uval_in;
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in_settable, max_size_increment_allowed);
	}
	else if(dti->type == TUPLE && dti == dti_in) // for TUPLE both must be same types
		return set_element_in_tuple(tpl_d, pa, tupl, &uval_in, max_size_increment_allowed);
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

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// if the child nested in middle is null, then we can not get element count of its child
		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return 0;

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		void* child_data = (void*) get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

int can_expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
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

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// if the child nested in middle is null, then we can not expand its child
		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return 0;

		// inner_most_dti is variable element_count, so it is bydefault a variable sized data type
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

int expand_element_count_for_element_in_tuple_INTERNAL(const data_type_info* dti, positional_accessor pa, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// loop termination cases
	{
		// result is self
		if(IS_SELF(pa))
			return expand_container(dti, data, index, slots, max_size_increment_allowed);
	}

	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if the child nested in middle is null, then we can not expand its child
	if(is_containee_null_in_container(dti, data, pa.positions[0]))
		return 0;

	// inner_most_dti is variable element_count, so it is bydefault a variable sized data type
	max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - get_size_for_type_info(dti, data));

	// then move child of current dti to the end
	move_variable_sized_containee_to_end_of_container(dti, data, pa.positions[0]);

	const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
	void* child_data = (void*) get_pointer_to_containee_from_container(dti, data, pa.positions[0]);

	uint32_t container_old_size = get_size_for_type_info(dti, data);
	uint32_t child_old_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);

	int result = expand_element_count_for_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, index, slots, max_size_increment_allowed);

	if(result)
	{
		uint32_t child_new_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);
		overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
	}

	return result;
}

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return expand_element_count_for_element_in_tuple_INTERNAL(dti, pa, data, index, slots, max_size_increment_allowed);
}

int can_discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl, uint32_t index, uint32_t slots)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
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

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		// if the child nested in middle is null, then we can not discard from its child
		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return 0;

		// inner_most_dti is variable element_count, so it is by default a variable sized data type

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

int discard_elements_from_element_in_tuple_INTERNAL(const data_type_info* dti, positional_accessor pa, void* data, uint32_t index, uint32_t slots)
{
	// loop termination cases
	{
		// result is self
		if(IS_SELF(pa))
			return discard_from_container(dti, data, index, slots);
	}

	if(!is_container_type_info(dti))
		return 0;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if the child nested in middle is null, then we can not discard from its child
	if(is_containee_null_in_container(dti, data, pa.positions[0]))
		return 0;

	// inner_most_dti is variable element_count, so it is by default a variable sized data type

	// then move child of current dti to the end
	move_variable_sized_containee_to_end_of_container(dti, data, pa.positions[0]);

	const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
	void* child_data = (void*) get_pointer_to_containee_from_container(dti, data, pa.positions[0]);

	uint32_t container_old_size = get_size_for_type_info(dti, data);
	uint32_t child_old_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);

	int result = discard_elements_from_element_in_tuple_INTERNAL(child_dti, NEXT_POSITION(pa), child_data, index, slots);

	if(result)
	{
		uint32_t child_new_size = get_size_of_containee_from_container(dti, data, pa.positions[0]);
		overwrite_size_for_container_type_info_with_size_in_prefix(dti, data, container_old_size - child_old_size + child_new_size);
	}

	return result;
}

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	return discard_elements_from_element_in_tuple_INTERNAL(dti, pa, data, index, slots);
}

// compare and hash functions
int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const void* tup2, const tuple_def* tpl_d2, positional_accessor pa2)
{
	// if the element is not accessible, then fail
	const data_type_info* dti1 = get_type_info_for_element_from_tuple(tpl_d1, pa1);
	if(dti1 == NULL)
		return -2;

	// if the element is not accessible, then fail
	const data_type_info* dti2 = get_type_info_for_element_from_tuple(tpl_d2, pa2);
	if(dti2 == NULL)
		return -2;

	// get the user value for this element
	const user_value uval1 = get_value_from_element_from_tuple(tpl_d1, pa1, tup1);

	// get the user value for this element
	const user_value uval2 = get_value_from_element_from_tuple(tpl_d2, pa2, tup2);

	// TODO : handle logic for custom compare function

	return compare_user_value(&uval1, dti1, &uval2, dti2);
}

int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const positional_accessor* element_ids1, const void* tup2, const tuple_def* tpl_d2, const positional_accessor* element_ids2, const compare_direction* cmp_dir, uint32_t element_count)
{
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

uint64_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, positional_accessor pa, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	// if the element is not accessible, then fail
	const data_type_info* dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(dti == NULL)
		return 0;

	// get the user value for this element
	const user_value uval = get_value_from_element_from_tuple(tpl_d, pa, tup);

	// TODO : handle logic for custom hash function

	return hash_user_value(&uval, dti, hash_func);
}

uint64_t hash_tuple(const void* tup, const tuple_def* tpl_d, const positional_accessor* element_ids, uint64_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count)
{
	uint64_t hash_value = 0;
	if(element_ids == NULL)
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, STATIC_POSITION(i), hash_func);
	}
	else
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, element_ids[i], hash_func);
	}
	return hash_value;
}

// print function

void print_tuple(const void* tup, const tuple_def* tpl_d)
{
	print_data_for_data_type_info(tpl_d->type_info, tup);
	printf("\n");
}
