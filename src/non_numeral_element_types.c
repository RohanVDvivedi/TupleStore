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
	const char* s1; uint32_t s1_max_len;
	const char* s2; uint32_t s2_max_len;

	if(ele_d_1->type == STRING)
	{
		s1 = e1;
		s1_max_len = ele_d_1->size;
	}
	else // else it is var_string
	{
		s1 = get_data_for_variable_sized_non_numeral_element(e1, ele_d_1);
		s1_max_len = get_data_size_for_variable_sized_non_numeral_element(e1, ele_d_1);
	}

	if(ele_d_2->type == STRING)
	{
		s2 = e2;
		s2_max_len = ele_d_2->size;
	}
	else // else it is var_string
	{
		s2 = get_data_for_variable_sized_non_numeral_element(e2, ele_d_2);
		s2_max_len = get_data_size_for_variable_sized_non_numeral_element(e2, ele_d_2);
	}

	uint32_t min_of_max_lens = min(s1_max_len, s2_max_len);

	int compare = strncmp(s1, s2, min_of_max_lens);

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else
	{
		uint32_t s1_len = get_string_length_for_string_type_element(e1, ele_d_1);
		uint32_t s2_len = get_string_length_for_string_type_element(e2, ele_d_2);
		if(s1_len > s2_len)
			compare = 1;
		else if(s1_len < s2_len)
			compare = -1;
	}

	return compare;
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

static void set_variable_sized_non_numeral_element_INTERNAL(void* e, const element_def* ele_d, const void* data, uint32_t data_size)
{
	write_uint32(e, ele_d->size_specifier_prefix_size, data_size);
	memmove(e + ele_d->size_specifier_prefix_size, data, data_size);
}

void set_variable_sized_non_numeral_element(void* e, const element_def* ele_d, const user_value* uval)
{
	set_variable_sized_non_numeral_element_INTERNAL(e, ele_d, uval->data, uval->data_size);
}

static void set_string_OR_blob_element_INTERNAL(void* e, const element_def* ele_d, const void* data, uint32_t data_size)
{
	if(is_fixed_sized_element_def(ele_d)) // for STRING or BLOB
	{
		uint32_t bytes_to_write = min(ele_d->size, data_size);

		memmove(e, data, bytes_to_write);

		// reset the unused bytes here
		if(bytes_to_write < ele_d->size)
			memset(e + bytes_to_write, 0, ele_d->size - bytes_to_write);
	}
	else // it is a VAR_BLOB or VAR_STRING
		set_variable_sized_non_numeral_element_INTERNAL(e, ele_d, data, data_size);
}

void set_string_OR_blob_element(void* e, const element_def* ele_d, const user_value* uval)
{
	set_string_OR_blob_element_INTERNAL(e, ele_d, uval->data, uval->data_size);
}