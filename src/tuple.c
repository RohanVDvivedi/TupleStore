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

int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL)
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

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
		if(is_variable_sized_type_info(inner_most_dti))
			move_variable_sized_containee_to_end_of_container(dti, data, pa.positions[0]);

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		void* child_data = (void*) get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed);

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

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	while(1)
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
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
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

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots)
{
	const data_type_info* inner_most_dti = get_type_info_for_element_from_tuple(tpl_d, pa);
	if(inner_most_dti == NULL || !has_variable_element_count_for_container_type_info(inner_most_dti))
		return 0;

	const data_type_info* dti = tpl_d->type_info;
	void* data = tupl;

	while(1)
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
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
}

// compare and hash functions
int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, positional_accessor pa1, const void* tup2, const tuple_def* tpl_d2, positional_accessor pa2)
{
	// TODO
	/*const element_def* ele_d1 = get_element_def_by_id(tpl_d1, index1);
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
		return compare_elements(e1, get_element_def_by_id(tpl_d1, index1), e2, get_element_def_by_id(tpl_d2, index2));*/
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
	const data_type_info* dti = tpl_d->type_info;
	const void* data = tup;

	while(1)
	{
		// loop termination cases
		{
			// result is self
			if(IS_SELF(pa))
				return hash_data_for_type_info(dti, data, hash_func);

			// result is self's some child
			if(pa.positions_length == 1)
				return hash_containee_in_container(dti, data, pa.positions[0], hash_func);
		}

		if(!is_container_type_info(dti))
			return 0;

		if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
			return 0;

		if(is_containee_null_in_container(dti, data, pa.positions[0]))
			return 0;

		const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
		const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
		dti = child_dti;
		data = child_data;
		pa = NEXT_POSITION(pa);
	}

	return 0;
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
