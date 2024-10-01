#include<tuple_def.h>

#include<bitmap.h>

#include<page_layout_util.h>

int initialize_tuple_size_def(tuple_size_def* tuple_size_d, data_type_info* dti)
{
	// can not create a tuple_size_def for BIT_FIELD type
	if(dti->type == BIT_FIELD)
		return 0;

	// must be able to finalize the type_info
	if(!finalize_type_info(dti))
		return 0;

	tuple_size_d->is_variable_sized = is_variable_sized_type_info(dti);

	if(!tuple_size_d->is_variable_sized)
	{
		tuple_size_d->size = dti->size;
		if(tuple_size_d->size == 0) // fixed sized data type must never be 0 sized
			return 0;
		return 1;
	}

	// a variable sized data type has both min_size and max_size
	tuple_size_d->min_size = dti->min_size;
	tuple_size_d->max_size = dti->max_size;

	tuple_size_d->has_size_in_prefix = has_size_in_its_prefix_for_container_type_info(dti);
	tuple_size_d->has_element_count_in_pefix = has_element_count_in_its_prefix_for_container_type_info(dti);

	// a variable sized type must either have size or element_count in prefix
	if(!tuple_size_d->has_size_in_prefix && !tuple_size_d->has_element_count_in_pefix)
		return 0;

	// if it already has size in prefix, we are done
	if(tuple_size_d->has_size_in_prefix)
		return 1;

	// else we need to populate the containee type info
	// it is either a variable sized string, variable sized blob OR variable element_count array of fixed sized elements
	// either way the containee is fixed sized	

	data_type_info* containee_type_info = dti->containee;

	tuple_size_d->does_containee_need_is_valid_bit_in_prefix = needs_is_valid_bit_in_prefix_bitmap(containee_type_info);

	tuple_size_d->is_containee_bit_field = (containee_type_info->type == BIT_FIELD);

	if(tuple_size_d->is_containee_bit_field)
		tuple_size_d->containee_bit_field_size = containee_type_info->bit_field_size;
	else
		tuple_size_d->containee_size = containee_type_info->size;

	return 1;
}

uint32_t get_tuple_size_using_tuple_size_def(const tuple_size_def* tpl_sz_d, const void* tupl);

int is_variable_sized_tuple_size_def(const tuple_size_def* tuple_size_d);

uint32_t initialize_minimal_tuple_for_tuple_size_info(const tuple_size_def* tpl_sz_d, void* tupl);

int initialize_tuple_def(tuple_def* tuple_d, data_type_info* dti);

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);

int is_variable_sized_tuple_def(const tuple_def* tuple_d);

#include<stdio.h>

void print_tuple_size_def(const tuple_size_def* tuple_size_d);

void print_tuple_def(const tuple_def* tuple_d);