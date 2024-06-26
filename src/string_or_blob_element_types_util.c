#include<string_or_blob_element_types_util.h>

#include<serial_int.h>

#include<cutlery_stds.h>

#include<string.h>

int is_string_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == STRING) || (ele_d->type == VAR_STRING);
}

int is_blob_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == BLOB) || (ele_d->type == VAR_BLOB);
}

int is_string_OR_blob_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == STRING) || (ele_d->type == BLOB) || (ele_d->type == VAR_STRING) || (ele_d->type == VAR_BLOB);
}

int is_fixed_sized_string_OR_blob_element_def(const element_def* ele_d)
{
	return (ele_d->type == STRING) || (ele_d->type == BLOB);
}

int is_variable_sized_string_OR_blob_element_def(const element_def* ele_d)
{
	return (ele_d->type == VAR_STRING) || (ele_d->type == VAR_BLOB);
}

static uint32_t get_data_size_for_string_OR_blob_element(const void* e, const element_def* ele_d)
{
	if(is_variable_sized_string_OR_blob_element_def(ele_d))	// for VAR_STRING and VAR_BLOB
		return deserialize_uint32(e, ele_d->size_specifier_prefix_size);
	else // STRING and BLOB
		return ele_d->size;
}

static const void* get_data_for_string_OR_blob_element(const void* e, const element_def* ele_d)
{
	if(is_variable_sized_string_OR_blob_element_def(ele_d))	// for VAR_STRING and VAR_BLOB
		return e + ele_d->size_specifier_prefix_size;
	else // STRING and BLOB
		return e;
}

uint32_t get_element_size_for_string_OR_blob_element(const void* e, const element_def* ele_d)
{
	if(is_variable_sized_string_OR_blob_element_def(ele_d))
		return ele_d->size_specifier_prefix_size + get_data_size_for_string_OR_blob_element(e, ele_d);
	else
		return ele_d->size;
}

uint32_t get_element_size_from_user_value_for_string_OR_blob_element(const user_value* uval, const element_def* ele_d)
{
	if(is_fixed_sized_string_OR_blob_element_def(ele_d)) // STRING and BLOB
		return ele_d->size;
	else // VAR_STRING and VAR_BLOB
	{
		if(is_user_value_NULL(uval))
			return 0;
		return ele_d->size_specifier_prefix_size + uval->data_size;
	}
}

uint32_t get_string_length_for_string_type_element(const void* e, const element_def* ele_d)
{
	// you must provide only a STRING or VAR_STRING to this function
	return strnlen(get_data_for_string_OR_blob_element(e, ele_d), get_data_size_for_string_OR_blob_element(e, ele_d));
}

int compare_string_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	const char* s1 = get_data_for_string_OR_blob_element(e1, ele_d_1);
	uint32_t s1_len = get_string_length_for_string_type_element(e1, ele_d_1);

	const char* s2 = get_data_for_string_OR_blob_element(e2, ele_d_2);
	uint32_t s2_len = get_string_length_for_string_type_element(e2, ele_d_2);

	// strncmp performs compares unsigned char comparison
	int compare = strncmp(s1, s2, min(s1_len, s2_len));

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else
	{
		if(s1_len > s2_len)
			compare = 1;
		else if(s1_len < s2_len)
			compare = -1;
	}

	return compare;
}

int compare_blob_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	const char* b1 = get_data_for_string_OR_blob_element(e1, ele_d_1);
	uint32_t b1_len = get_data_size_for_string_OR_blob_element(e1, ele_d_1);

	const void* b2 = get_data_for_string_OR_blob_element(e2, ele_d_2);
	uint32_t b2_len = get_data_size_for_string_OR_blob_element(e2, ele_d_2);

	// find min of their lengths
	uint32_t min_len = min(b1_len, b2_len);

	// compare only their min_len number of bytes
	// memcmp performs compares unsigned char comparison
	int compare = memcmp(b1, b2, min_len);

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else if((compare == 0) && (b1_len != b2_len))
	{
		// in dictionary ordering if 1 blob is a prefix of the other
		// then the larger blob comes latter in the order
		if(b1_len > b2_len)
			compare = -1;
		else if(b1_len > b2_len)
			compare = 1;
	}

	return compare;
}

uint64_t hash_string_OR_blob_type_element(const void* e, const element_def* ele_d, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	if(is_string_type_element_def(ele_d))
		return hash_func(get_data_for_string_OR_blob_element(e, ele_d), get_string_length_for_string_type_element(e, ele_d));
	else
		return hash_func(get_data_for_string_OR_blob_element(e, ele_d), get_data_size_for_string_OR_blob_element(e, ele_d));
}

static void set_string_OR_blob_element_INTERNAL(void* e, const element_def* ele_d, const void* data, uint32_t data_size)
{
	if(is_fixed_sized_element_def(ele_d)) // for STRING or BLOB
	{
		uint32_t bytes_to_write = min(ele_d->size, data_size);

		memory_move(e, data, bytes_to_write);

		// reset the unused bytes here
		if(bytes_to_write < ele_d->size)
			memory_set(e + bytes_to_write, 0, ele_d->size - bytes_to_write);
	}
	else // it is a VAR_BLOB or VAR_STRING
	{
		// set the data size in the specified location, it will be be followed by data
		serialize_uint32(e, ele_d->size_specifier_prefix_size, data_size);

		// copy the actual data now
		memory_move(e + ele_d->size_specifier_prefix_size, data, data_size);
	}
}

void set_string_OR_blob_element(void* e, const element_def* ele_d, const user_value* uval)
{
	// ensure that user value is not NULL
	if(is_user_value_NULL(uval))
		return;

	set_string_OR_blob_element_INTERNAL(e, ele_d, uval->data, uval->data_size);
}

int set_string_OR_blob_element_from_element(void* e, const element_def* ele_d, const void* e_from, const element_def* ele_d_from)
{
	if( !( (is_string_type_element_def(ele_d) && is_string_type_element_def(ele_d_from)) 
		|| (is_blob_type_element_def(ele_d) && is_blob_type_element_def(ele_d_from)) ) )
		return 0;

	const void* data = get_data_for_string_OR_blob_element(e_from, ele_d_from);

	uint32_t data_size;
	if(is_string_type_element_def(ele_d_from))
		data_size = get_string_length_for_string_type_element(e_from, ele_d_from);
	else // BLOB / VAR_BLOB
		data_size = get_data_size_for_string_OR_blob_element(e_from, ele_d_from);

	set_string_OR_blob_element_INTERNAL(e, ele_d, data, data_size);

	return 1;
}

user_value get_value_from_string_OR_blob_element(const void* e, const element_def* ele_d)
{
	user_value uval = {};

	uval.data = get_data_for_string_OR_blob_element(e, ele_d);

	if(is_string_type_element_def(ele_d))
		uval.data_size = get_string_length_for_string_type_element(e, ele_d);
	else
		uval.data_size = get_data_size_for_string_OR_blob_element(e, ele_d);

	return uval;
}

user_value get_MIN_value_for_string_OR_blob_element_def(const element_def* ele_d)
{
	// gives out a length of 0 bytes long string or blob
	return (*ZERO_USER_VALUE);
}