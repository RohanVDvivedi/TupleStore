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

int initialize_tuple_def(tuple_def* tuple_d, data_type_info* dti)
{
	int result = initialize_tuple_size_def(&(tuple_d->size_def), dti);
	if(result)
		tuple_d->type_info = dti;
	return result;
}

uint32_t get_tuple_size_using_tuple_size_def(const tuple_size_def* tpl_sz_d, const void* tupl)
{
	// if fixed sized return size
	if(!tpl_sz_d->is_variable_sized)
		return tpl_sz_d->size;

	// else we know it is variable sized

	// if has size in prefix read that, and return it
	if(tpl_sz_d->has_size_in_prefix)
		return read_value_from_page(tupl, tpl_sz_d->max_size);

	// else read element_count
	uint32_t element_count = read_value_from_page(tupl, tpl_sz_d->max_size);

	// it has element_count in its prefix but not its size

	if(tpl_sz_d->is_containee_bit_field)
		return get_value_size_on_page(tpl_sz_d->max_size) + bitmap_size_in_bytes(element_count * (tpl_sz_d->does_containee_need_is_valid_bit_in_prefix + tpl_sz_d->containee_bit_field_size));
	else
		return get_value_size_on_page(tpl_sz_d->max_size) + bitmap_size_in_bytes(element_count * tpl_sz_d->does_containee_need_is_valid_bit_in_prefix) + (element_count * tpl_sz_d->containee_size);
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	return get_tuple_size_using_tuple_size_def(&(tpl_d->size_def), tupl);
}

int is_variable_sized_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	return tuple_size_d->is_variable_sized;
}

int is_variable_sized_tuple_def(const tuple_def* tuple_d)
{
	return is_variable_sized_tuple_size_def(&(tuple_d->size_def));
}

// this is left here to show that this could be done but should not be done
uint32_t initialize_minimal_tuple_for_tuple_size_info(const tuple_size_def* tpl_sz_d, void* tupl)
{
	if(!tpl_sz_d->is_variable_sized)
	{
		memory_set(tupl, 0, tpl_sz_d->size);
		return tpl_sz_d->size;
	}
	else
	{
		memory_set(tupl, 0, tpl_sz_d->min_size);
		// if it has size set it to min_size, element_count if exists on the tupl is set to 0 by the above statement
		if(tpl_sz_d->has_size_in_prefix)
			write_value_to_page(tupl, tpl_sz_d->max_size, tpl_sz_d->min_size);
		return tpl_sz_d->min_size;
	}
}

#include<stdio.h>

void print_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	printf("is_variable_sized : %d\n", tuple_size_d->is_variable_sized);
	if(!tuple_size_d->is_variable_sized)
	{
		printf("size = %"PRIu32"\n", tuple_size_d->size);
		return;
	}

	printf("min_size : %"PRIu32"\n", tuple_size_d->min_size);
	printf("max_size : %"PRIu32"\n", tuple_size_d->max_size);

	printf("has_size_in_prefix : %d\n", tuple_size_d->has_size_in_prefix);
	printf("has_element_count_in_pefix : %d\n", tuple_size_d->has_element_count_in_pefix);

	if(tuple_size_d->has_size_in_prefix)
		return;

	printf("does_containee_need_is_valid_bit_in_prefix : %d\n", tuple_size_d->does_containee_need_is_valid_bit_in_prefix);
	printf("is_containee_bit_field : %d\n", tuple_size_d->is_containee_bit_field);

	if(tuple_size_d->is_containee_bit_field)
		printf("containee_bit_field_size : %"PRIu32"\n", tuple_size_d->containee_bit_field_size);
	else
		printf("containee_size : %"PRIu32"\n", tuple_size_d->containee_size);
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("size_def : ");
	print_tuple_size_def(&(tuple_d->size_def));
	printf("type_info : ");
	print_type_info(tuple_d->type_info);
}