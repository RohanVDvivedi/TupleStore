#include<tuple.h>

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	initialize_minimal_data_for_type_info(tpl_d->type_info, tupl);
}

static const user_value get_value_from_element_from_type_info(const data_type_info* dti, positional_accessor pa, const void* data)
{
	// result is self
	if(IS_SELF(pa))
		return get_user_value_for_type_info(dti, data);

	// resul is self's some child
	if(pa.positions_length == 1)
		return get_user_value_to_containee_from_container(dti, data, pa.positions[0]);

	// none of the above case suffice, then recurse

	if(!is_container_type_info(dti))
		return *OUT_OF_BOUNDS_USER_VALUE;

	if(pa.positions[0] >= get_element_count_for_container_type_info(dti, data))
		return *OUT_OF_BOUNDS_USER_VALUE;

	if(is_containee_null_in_container(dti, data, pa.positions[0]))
		return *NULL_USER_VALUE;

	const data_type_info* child_dti = get_data_type_info_for_containee_of_container(dti, data, pa.positions[0]);
	const void* child_data = get_pointer_to_containee_from_container(dti, data, pa.positions[0]);
	return get_value_from_element_from_type_info(child_dti, NEXT_POSITION(pa), child_data);
}

const user_value get_value_from_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl)
{
	return get_value_from_element_from_type_info(tpl_d->type_info, pa, tupl);
}

const data_type_info* get_type_info_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

int can_set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

int set_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const user_value* value, uint32_t max_size_increment_allowed);

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, const tuple_def* tpl_d_in, positional_accessor pa_in, const void* tupl_in, uint32_t max_size_increment_allowed);

uint32_t get_element_count_for_element_from_tuple(const tuple_def* tpl_d, positional_accessor pa, const void* tupl);

int can_expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int expand_element_count_for_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

int discard_elements_from_element_in_tuple(const tuple_def* tpl_d, positional_accessor pa, void* tupl, uint32_t index, uint32_t slots);

// compare and hash functions

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

int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const uint32_t* element_ids1, const void* tup2, const tuple_def* tpl_d2, const uint32_t* element_ids2, const compare_direction* cmp_dir, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
	{
		compare = compare_elements_of_tuple(tup1, tpl_d1, ((element_ids1 == NULL) ? i : element_ids1[i]), tup2, tpl_d2, ((element_ids2 == NULL) ? i : element_ids2[i]));
		
		// if cmp_dir is not NULL, then compare in default direction of the element
		if(cmp_dir != NULL)
			compare = compare * cmp_dir[i];
	}
	return compare;
}

uint64_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	const void* e = get_element_from_tuple(tpl_d, index, tup);

	// hashing a NULL element returns a 0
	if(e == NULL)
		return 0;

	const element_def* ele_d = get_element_def_by_id(tpl_d, index);
	return hash_element(e, ele_d, hash_func);
}

uint64_t hash_tuple(const void* tup, const tuple_def* tpl_d, const uint32_t* element_ids, uint64_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count)
{
	uint64_t hash_value = 0;
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

// print function

void print_tuple(const void* tup, const tuple_def* tpl_d)
{
	print_data_for_data_type_info(tpl_d->type_info, tup);
	printf("\n");
}
