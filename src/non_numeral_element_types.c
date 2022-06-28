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

const void* get_data_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d)
{
	return e + ele_d->size_specifier_prefix_size;
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
		return get_data_size_for_variable_sized_non_numeral_element(e, ele_d);
}

int compare_string_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{

}

int compare_blob_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	const void* b1;	uint32_t b1_len;
	const void* b2;	uint32_t b2_len;

	if(ele_d_1->type == BLOB)
	{
		b1 = e1;
		b1_len = ele_d_1->size;
	}
	else // else it is var_blob
	{
		b1 = get_data_for_variable_sized_non_numeral_element(e1, ele_d_1);
		b1_len = get_data_size_for_variable_sized_non_numeral_element(e1, ele_d_1);
	}

	if(ele_d_2->type == BLOB)
	{
		b2 = e2;
		b2_len = ele_d_2->size;
	}
	else // else it is var_blob
	{
		b2 = get_data_for_variable_sized_non_numeral_element(e2, ele_d_2);
		b2_len = get_data_size_for_variable_sized_non_numeral_element(e2, ele_d_2);
	}

	// find min of their lengths
	uint32_t min_len = min(b1_len, b2_len);

	// compare only their min_len number of bytes
	int compare = memcmp(b1, b2, min_len);

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else if((compare == 0) && (b1_len != b2_len))
	{
		// in dictionary ordering if 1 string is a prefix of the other
		// then the larger string comes latter in the order
		if(b1_len > b2_len)
			compare = -1;
		else if(b1_len > b2_len)
			compare = 1;
	}

	return compare;
}