#include<non_numeral_element_types.h>

#include<int_accesses.h>

#include<string.h>

int is_string_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == STRING) || (ele_d->type == VAR_STRING);
}

int is_blob_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == BLOB) || (ele_d->type == VAR_BLOB);
}

int is_variable_sized_non_numeral_element_def(const element_def* ele_d)
{
	return (ele_d->type == VAR_STRING) || (ele_d->type == VAR_BLOB);
}

uint32_t get_data_size_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d)
{
	return read_uint32(e, ele_d->size_specifier_prefix_size);
}

uint32_t get_element_size_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d)
{
	return ele_d->size_specifier_prefix_size + get_data_size_for_variable_sized_non_numeral_element(e, ele_d);
}

uint32_t get_string_length_for_string_type_element(const void* e, const element_def* ele_d)
{
	if(ele_d->type == STRING)
		return strnlen(e, ele_d->size);
	else // else it must be VAR_STRING
		get_data_size_for_variable_sized_non_numeral_element(e, ele_d);
}