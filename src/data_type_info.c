#include<tuplestore/data_type_info.h>
#include<tuplestore/page_layout_util.h>

#include<serint/serial_int.h>
#include<serint/large_uints.h>
#include<tuplestore/float_accesses.h>

#include<cutlery/bitmap.h>

#include<string.h>
#include<stdio.h>

char const * const types_as_string[] = {
								"BIT_FIELD",
								"UINT",
								"INT",
								"FLOAT",
								"LARGE_UINT",
								"STRING",
								"BLOB",
								"TUPLE",
								"ARRAY",
							};

uint32_t find_containee_using_field_name_in_tuple_type_info(const data_type_info* dti, const char* field_name)
{
	// fail if not a tuple
	if(dti->type != TUPLE)
		return -1;

	// it is TUPLE hence the element_count is fixed

	for(uint32_t i = 0; i < dti->element_count; i++)
	{
		if(strcmp(field_name, dti->containees[i].field_name) == 0)
			return i;
	}

	return -1;
}

int finalize_type_info(data_type_info* dti)
{
	// no need to finalize again
	if(dti->is_finalized)
		return 1;

	switch(dti->type)
	{
		case BIT_FIELD :
		{
			if(dti->bit_field_size < 1 || 64 < dti->bit_field_size)
				return 0;
			dti->is_variable_sized = 0;
			break;
		}

		case UINT :
		case INT :
		{
			if(dti->size < 1 || 8 < dti->size)
				return 0;
			dti->is_variable_sized = 0;
			break;
		}

		case FLOAT :
		{
			if((dti->size != sizeof(float)) && (dti->size != sizeof(double)) && (dti->size != sizeof(long double)))
				return 0;
			dti->is_variable_sized = 0;
			break;
		}

		case LARGE_UINT :
		{
			if(dti->size < 1 || get_max_bytes_uint256() < dti->size)
				return 0;
			dti->is_variable_sized = 0;
			break;
		}

		case STRING :
		case BLOB :
		{
			dti->containee = UINT_NON_NULLABLE[1];
			if(!finalize_type_info(dti->containee))
				return 0;
			dti->is_variable_sized = dti->has_variable_element_count;
			if(!dti->has_variable_element_count && dti->element_count == 0) // for a fixed element count container the element_count must never be 0
				return 0;
			dti->prefix_bitmap_size_in_bits = 0; // will always be zero here
			if(dti->is_variable_sized)
			{
				dti->min_size = get_value_size_on_page(dti->max_size); // an empty string or blob
				if(dti->min_size > dti->max_size)
					return 0;
			}
			else
				dti->size = dti->element_count;
			break;
		}

		case TUPLE :
		{
			// you can never finalize a TUPLE type with no elements
			if(dti->element_count == 0)
				return 0;

			// initialize the attributes
			dti->is_variable_sized = 0;
			dti->prefix_bitmap_size_in_bits = 0;

			// tuple will never have variable element count
			dti->has_variable_element_count = 0;

			// figure out 2 things in this loop
			// whether the tuple is_variable_sized
			// and the number of the bits required in prefix_bitmap, and assign bits to the bit_fields in prefix_bitmap
			for(uint32_t i = 0; i < dti->element_count; i++)
			{
				data_type_info* containee_type_info = dti->containees[i].al.type_info;
				data_position_info* containee_pos_info = dti->containees + i;

				if(!finalize_type_info(containee_type_info))
					return 0;

				// if the element is variable_sized, then mark the tuple as variable sized
				if(is_variable_sized_type_info(containee_type_info))
					dti->is_variable_sized = 1;

				// give this element an is_NULL bit in prefix_bitmap only if it needs it
				if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
				{
					// assign bit_offset_to_is_valid_bit and increment the prefix_bitmap_size_in_bits
					containee_pos_info->al.bit_offset_to_is_valid_bit = dti->prefix_bitmap_size_in_bits;
					dti->prefix_bitmap_size_in_bits += 1;
				}

				// assign bit offsets in prefix_bitmap, to the bit_fields
				if(containee_type_info->type == BIT_FIELD)
				{
					// assign bit_offset to bit_field and increment the prefix_bitmap_size_in_bits
					containee_pos_info->al.bit_offset_in_prefix_bitmap = dti->prefix_bitmap_size_in_bits;
					dti->prefix_bitmap_size_in_bits += containee_type_info->bit_field_size;
				}
			}

			dti->size = 0; // alse sets min_size to 0

			// allocate space for storing tuple_size for variable sized tuple_def
			if(dti->is_variable_sized)
				dti->min_size += get_value_size_on_page(dti->max_size);

			// add prefix_bitmap's size in bytes to tuple_sed size 
			dti->size += bitmap_size_in_bytes(dti->prefix_bitmap_size_in_bits);

			// now we compute the offsets (and the offsets to their byte_offsets) for all the elements
			for(uint32_t i = 0; i < dti->element_count; i++)
			{
				data_type_info* containee_type_info = dti->containees[i].al.type_info;
				data_position_info* containee_pos_info = dti->containees + i;

				// all the work needed for BIT_FIELD is already done, hence we can skip it
				if(containee_type_info->type == BIT_FIELD)
					continue;

				if(is_variable_sized_type_info(containee_type_info))
				{
					containee_pos_info->al.byte_offset_to_byte_offset = dti->min_size;
					dti->min_size += get_value_size_on_page(dti->max_size);
				}
				else
				{
					containee_pos_info->al.byte_offset = dti->size;
					dti->size += containee_type_info->size;
				}
			}

			if(is_variable_sized_type_info(dti) && dti->min_size > dti->max_size)
				return 0;

			break;
		}

		case ARRAY :
		{
			if(!finalize_type_info(dti->containee))
				return 0;
			dti->is_variable_sized = dti->has_variable_element_count || is_variable_sized_type_info(dti->containee);
			if(!dti->has_variable_element_count && dti->element_count == 0) // for a fixed element count container the element_count must never be 0
				return 0;
			if(!dti->has_variable_element_count) // prefix_bitmap_size_in_bits is only defined for fixed element count containers
			{
				if(dti->containee->type == BIT_FIELD)
					dti->prefix_bitmap_size_in_bits = dti->element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size);
				else if(!is_variable_sized_type_info(dti->containee))
					dti->prefix_bitmap_size_in_bits = dti->element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee));
				else
					dti->prefix_bitmap_size_in_bits = 0;
			}
			if(!dti->is_variable_sized)
			{
				// is fixed sized, hence can not have variable element count
				// the elements must be bit fields or fixed sized elements
				if(dti->containee->type == BIT_FIELD)
					dti->size = bitmap_size_in_bytes(dti->prefix_bitmap_size_in_bits);
				else
					dti->size = bitmap_size_in_bytes(dti->prefix_bitmap_size_in_bits) + dti->element_count * dti->containee->size;
			}
			else
			{
				// get the default prefix size, only requires us to know if it needs size or element_count in prefix or not.
				dti->min_size = get_offset_to_prefix_bitmap_for_container_type_info(dti);
				// for variable element count, make element count to 0 and we are done
				// but for fixed length element we need space for its elements or their offsets
				if(!dti->has_variable_element_count)
				{
					if(dti->containee->type == BIT_FIELD)
						dti->min_size += bitmap_size_in_bytes((needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size) * dti->element_count);
					else if(!is_variable_sized_type_info(dti->containee))
						dti->min_size += bitmap_size_in_bytes(needs_is_valid_bit_in_prefix_bitmap(dti->containee) * dti->element_count) + (dti->containee->size * dti->element_count);
					else
						dti->min_size += (get_value_size_on_page(dti->max_size) * dti->element_count);
				}
				if(dti->min_size > dti->max_size)
					return 0;
			}
		}
	}

	dti->is_finalized = 1;

	return 1;
}

static uint32_t get_byte_count_for_serialized_type_name(const char* type_name)
{
	uint32_t type_name_length = strnlen(type_name, 64) + 1;
	if(type_name_length > 64)
		type_name_length = 64;
	return type_name_length;
}

uint32_t get_byte_count_for_serialized_type_info(const data_type_info* dti)
{
	uint32_t bytes_consumed = 1;

	switch(dti->type)
	{
		case BIT_FIELD :
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
		{
			bytes_consumed += get_byte_count_for_serialized_type_name(dti->type_name);
			break;
		}
		case STRING :
		case BLOB :
		{
			bytes_consumed += 4;
			bytes_consumed += get_byte_count_for_serialized_type_name(dti->type_name);
			break;
		}
		case TUPLE :
		{
			if(!is_variable_sized_type_info(dti))
				bytes_consumed += 4;
			else
				bytes_consumed += 8;

			bytes_consumed += get_byte_count_for_serialized_type_name(dti->type_name);

			for(uint32_t i = 0; i < dti->element_count; i++)
			{
				bytes_consumed += get_byte_count_for_serialized_type_name(dti->containees[i].field_name); // field name is also 64 bytes, so it's serialization works same as type_name
				bytes_consumed += get_byte_count_for_serialized_type_info(dti->containees[i].al.type_info);
			}

			break;
		}
		case ARRAY :
		{
			if(!is_variable_sized_type_info(dti)) // fixed sized array implies fixed element count and containing fixed sized containee
				bytes_consumed += 4;
			else
			{
				if(!is_variable_element_count_container_type_info(dti))
					bytes_consumed += 4;

				bytes_consumed += 4;
			}

			bytes_consumed += get_byte_count_for_serialized_type_name(dti->type_name);

			bytes_consumed +=  get_byte_count_for_serialized_type_info(dti->containee);

			break;
		}
	}

	return bytes_consumed;
}

static uint32_t serialize_type_name(const char* type_name, unsigned char* holder)
{
	uint32_t type_name_length = strnlen(type_name, 64) + 1;
	if(type_name_length > 64)
		type_name_length = 64;
	strncpy((char*) holder, type_name, type_name_length);
	return type_name_length;
}

uint32_t serialize_type_info(const data_type_info* dti, void* data)
{
	unsigned char* serialized_bytes = data;
	uint32_t bytes_consumed = 1;

	switch(dti->type)
	{
		case BIT_FIELD :
		{
			if(is_nullable_type_info(dti))
				serialized_bytes[0] = 0 + dti->bit_field_size - 1;
			else
				serialized_bytes[0] = 64 + dti->bit_field_size - 1;

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case UINT :
		{
			if(is_nullable_type_info(dti))
				serialized_bytes[0] = 128 + dti->size - 1;
			else
				serialized_bytes[0] = 136 + dti->size - 1;

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case INT :
		{
			if(is_nullable_type_info(dti))
				serialized_bytes[0] = 144 + dti->size - 1;
			else
				serialized_bytes[0] = 152 + dti->size - 1;

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case FLOAT :
		{
			if(is_nullable_type_info(dti))
			{
				if(dti->size == sizeof(float))
					serialized_bytes[0] = 160;
				else if(dti->size == sizeof(double))
					serialized_bytes[0] = 161;
				else
					serialized_bytes[0] = 162;
			}
			else
			{
				if(dti->size == sizeof(float))
					serialized_bytes[0] = 163;
				else if(dti->size == sizeof(double))
					serialized_bytes[0] = 164;
				else
					serialized_bytes[0] = 165;
			}

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case LARGE_UINT :
		{
			if(is_nullable_type_info(dti))
				serialized_bytes[0] = 166 + dti->size - 1;
			else
				serialized_bytes[0] = 198 + dti->size - 1;

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case STRING :
		{
			if(!is_variable_sized_type_info(dti))
			{
				if(is_nullable_type_info(dti))
					serialized_bytes[0] = 230;
				else
					serialized_bytes[0] = 231;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;
			}
			else
			{
				serialized_bytes[0] = 232;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->max_size); bytes_consumed += 4;
			}

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case BLOB :
		{
			if(!is_variable_sized_type_info(dti))
			{
				if(is_nullable_type_info(dti))
					serialized_bytes[0] = 233;
				else
					serialized_bytes[0] = 234;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;
			}
			else
			{
				serialized_bytes[0] = 235;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->max_size); bytes_consumed += 4;
			}

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			break;
		}
		case TUPLE :
		{
			if(!is_variable_sized_type_info(dti))
			{
				if(is_nullable_type_info(dti))
					serialized_bytes[0] = 236;
				else
					serialized_bytes[0] = 237;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;
			}
			else
			{
				serialized_bytes[0] = 238;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->max_size); bytes_consumed += 4;
			}

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			for(uint32_t i = 0; i < dti->element_count; i++)
			{
				bytes_consumed += serialize_type_name(dti->containees[i].field_name, serialized_bytes + bytes_consumed); // field name is also 64 bytes, so it's serialization works same as type_name

				bytes_consumed += serialize_type_info(dti->containees[i].al.type_info, serialized_bytes + bytes_consumed);
			}

			break;
		}
		case ARRAY :
		{
			if(!is_variable_sized_type_info(dti)) // fixed sized array implies fixed element count and containing fixed sized containee
			{
				if(is_nullable_type_info(dti))
					serialized_bytes[0] = 239;
				else
					serialized_bytes[0] = 240;

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;
			}
			else
			{
				if(!is_variable_element_count_container_type_info(dti))
				{
					serialized_bytes[0] = 241;

					serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->element_count); bytes_consumed += 4;
				}
				else
				{
					serialized_bytes[0] = 242;
				}

				serialize_uint32(serialized_bytes + bytes_consumed, 4, dti->max_size); bytes_consumed += 4;
			}

			bytes_consumed += serialize_type_name(dti->type_name, serialized_bytes + bytes_consumed);

			bytes_consumed += serialize_type_info(dti->containee, serialized_bytes + bytes_consumed);

			break;
		}
	}

	return bytes_consumed;
}

// returns UINT32_MAX on error
static uint32_t deserialize_type_name(void* result, const void* data, uint32_t data_size)
{
	uint32_t type_name_length = strnlen(data, min(64, data_size));
	if(type_name_length != 64) // then it must have null terminator at the end
	{
		if(type_name_length != data_size)
		{
			if(((const char*)data)[type_name_length] != '\0') // this case must never occur
				return UINT32_MAX;
			else
				type_name_length += 1; // count the null terminator in the length to be copied
		}
		else // hitting data_size without null terminator is an error
			return UINT32_MAX;
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wstringop-truncation"
		strncpy(result, data, type_name_length);
	#pragma GCC diagnostic pop
	return type_name_length;
}

#include<stdlib.h>

data_type_info* deserialize_type_info(const void* data, uint32_t data_size, int* allocation_error)
{
	if(data_size < 1)
		return NULL;

	const unsigned char* serialized_bytes = data;
	uint32_t bytes_consumed = 1;

	if(serialized_bytes[0] <= 63)
	{
		uint32_t size = serialized_bytes[0] + 1;

		data_type_info dti = define_bit_field_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, BIT_FIELD_NULLABLE[size]))
			return BIT_FIELD_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 127)
	{
		uint32_t size = serialized_bytes[0] - 63;

		data_type_info dti = define_bit_field_non_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, BIT_FIELD_NON_NULLABLE[size]))
			return BIT_FIELD_NON_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 135)
	{
		uint32_t size = serialized_bytes[0] - 127;

		data_type_info dti = define_uint_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, UINT_NULLABLE[size]))
			return UINT_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 143)
	{
		uint32_t size = serialized_bytes[0] - 135;

		data_type_info dti = define_uint_non_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, UINT_NON_NULLABLE[size]))
			return UINT_NON_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 151)
	{
		uint32_t size = serialized_bytes[0] - 143;

		data_type_info dti = define_int_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, INT_NULLABLE[size]))
			return INT_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 159)
	{
		uint32_t size = serialized_bytes[0] - 151;

		data_type_info dti = define_int_non_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, INT_NON_NULLABLE[size]))
			return INT_NON_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 162)
	{
		uint32_t type_no = serialized_bytes[0] - 159;

		data_type_info dti = {};
		if(type_no == 1)
			dti = define_float_nullable_type("", float);
		else if(type_no == 2)
			dti = define_float_nullable_type("", double);
		else if(type_no == 3)
			dti = define_float_nullable_type("", long_double);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, FLOAT_float_NULLABLE))
			return FLOAT_float_NULLABLE;
		else if(are_identical_type_info(&dti, FLOAT_double_NULLABLE))
			return FLOAT_double_NULLABLE;
		else if(are_identical_type_info(&dti, FLOAT_long_double_NULLABLE))
			return FLOAT_long_double_NULLABLE;

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 165)
	{
		uint32_t type_no = serialized_bytes[0] - 162;

		data_type_info dti = {};
		if(type_no == 1)
			dti = define_float_non_nullable_type("", float);
		else if(type_no == 2)
			dti = define_float_non_nullable_type("", double);
		else if(type_no == 3)
			dti = define_float_non_nullable_type("", long_double);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, FLOAT_float_NULLABLE))
			return FLOAT_float_NULLABLE;
		else if(are_identical_type_info(&dti, FLOAT_double_NULLABLE))
			return FLOAT_double_NULLABLE;
		else if(are_identical_type_info(&dti, FLOAT_long_double_NULLABLE))
			return FLOAT_long_double_NULLABLE;

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 197)
	{
		uint32_t size = serialized_bytes[0] - 165;

		data_type_info dti = define_large_uint_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, LARGE_UINT_NULLABLE[size]))
			return LARGE_UINT_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 229)
	{
		uint32_t size = serialized_bytes[0] - 197;

		data_type_info dti = define_large_uint_non_nullable_type("", size);
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		if(are_identical_type_info(&dti, LARGE_UINT_NON_NULLABLE[size]))
			return LARGE_UINT_NON_NULLABLE[size];

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 232) // STRING
	{
		data_type_info dti = {};

		if(serialized_bytes[0] <= 231)
		{
			int is_nullable = (serialized_bytes[0] == 230);

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_fixed_length_string_type("", element_count, is_nullable);
		}
		else
		{
			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t max_size = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_variable_length_string_type("", max_size);
		}
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 235) // BLOB
	{
		data_type_info dti = {};

		if(serialized_bytes[0] <= 234)
		{
			int is_nullable = (serialized_bytes[0] == 233);

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_fixed_length_blob_type("", element_count, is_nullable);
		}
		else
		{
			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t max_size = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_variable_length_blob_type("", max_size);
		}
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 238) // TUPLE
	{
		data_type_info* dti_p = NULL;

		if(serialized_bytes[0] <= 237)
		{
			int is_nullable = (serialized_bytes[0] == 236);

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti_p = malloc(sizeof_tuple_data_type_info(element_count));
			if(dti_p == NULL)
			{
				(*allocation_error) = 1;
				return NULL;
			}

			initialize_tuple_data_type_info(dti_p, "", is_nullable, 0, element_count);
		}
		else
		{
			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t max_size = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti_p = malloc(sizeof_tuple_data_type_info(element_count));
			if(dti_p == NULL)
			{
				(*allocation_error) = 1;
				return NULL;
			}

			initialize_tuple_data_type_info(dti_p, "", 1, max_size, element_count);
		}
		uint32_t type_name_length = deserialize_type_name(dti_p->type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
		{
			// no containees initialized yet, hence no need to destroy the containees
			free(dti_p);
			return NULL;
		}
		else
			bytes_consumed += type_name_length;

		for(uint32_t i = 0; i < dti_p->element_count; i++)
		{
			uint32_t field_name_length = deserialize_type_name(dti_p->containees[i].field_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
			if(field_name_length == UINT32_MAX)
				goto DESTROY_ALL_CHILDREN_UNTIL_i_AND_FAIL;
			else
				bytes_consumed += field_name_length;

			dti_p->containees[i].al.type_info = deserialize_type_info(serialized_bytes + bytes_consumed, data_size - bytes_consumed, allocation_error);
			if(dti_p->containees[i].al.type_info == NULL)
				goto DESTROY_ALL_CHILDREN_UNTIL_i_AND_FAIL;
			else
				bytes_consumed += get_byte_count_for_serialized_type_info(dti_p->containees[i].al.type_info);

			// logic below this is to handle failure in the loop, so if you succeed until here continue
			continue;

			DESTROY_ALL_CHILDREN_UNTIL_i_AND_FAIL:;
			{
				// destroy all children of dti_p
				for(uint32_t j = 0; j < i; j++)
					destroy_non_static_type_info_recursively(dti_p->containees[j].al.type_info);
				free(dti_p);
				return NULL;
			}
		}

		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else if(serialized_bytes[0] <= 242) // ARRAY
	{
		data_type_info dti = {};

		if(serialized_bytes[0] <= 240)
		{
			int is_nullable = (serialized_bytes[0] == 239);

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_fixed_element_count_array_type("", element_count, 0, is_nullable, NULL);
		}
		else if(serialized_bytes[0] <= 241)
		{
			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t element_count = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t max_size = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_fixed_element_count_array_type("", element_count, max_size, 1, NULL);
		}
		else // 242
		{
			if(bytes_consumed + 4 > data_size)
				return NULL;
			uint32_t max_size = deserialize_uint32(serialized_bytes + bytes_consumed, 4); bytes_consumed += 4;

			dti = get_variable_element_count_array_type("", max_size, NULL);
		}
		uint32_t type_name_length = deserialize_type_name(dti.type_name, serialized_bytes + bytes_consumed, data_size - bytes_consumed);
		if(type_name_length == UINT32_MAX)
			return NULL;
		else
			bytes_consumed += type_name_length;

		dti.containee = deserialize_type_info(serialized_bytes + bytes_consumed, data_size - bytes_consumed, allocation_error);
		if(dti.containee == NULL)
			return NULL;

		data_type_info* dti_p = malloc(sizeof(data_type_info));
		if(dti_p == NULL)
		{
			(*allocation_error) = 1;
			// destroy containee
			destroy_non_static_type_info_recursively(dti.containee);
			return NULL;
		}
		(*dti_p) = dti;
		dti_p->is_static = 0; // since we are returning an allocated type_info it can not be static
		return dti_p;
	}
	else
		return NULL;
}

void destroy_non_static_type_info_recursively(data_type_info* dti)
{
	if(dti->is_static)
		return;

	switch(dti->type)
	{
		case BIT_FIELD :
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
		{
			free(dti);
			return;
		}
		case STRING :
		case BLOB :
		case ARRAY :
		{
			destroy_non_static_type_info_recursively(dti->containee);
			free(dti);
			return;
		}
		case TUPLE :
		{
			for(uint32_t i = 0; i < dti->element_count; i++)
				destroy_non_static_type_info_recursively(dti->containees[i].al.type_info);
			free(dti);
			return;
		}
	}
}

int are_identical_type_info(const data_type_info* dti1, const data_type_info* dti2)
{
	// if either of dti1 or dti2 is not finalized, we fail
	if(!dti1->is_finalized)
		return 0;
	if(!dti2->is_finalized)
		return 0;

	// if the passed pointers are equal, they are equal
	if(dti1 == dti2)
		return 1;

	// if their type or type_name are not equal, fail
	if(dti1->type != dti2->type)
		return 0;
	if(strcmp(dti1->type_name, dti2->type_name) != 0)
		return 0;

	// both must be either fixed sized or variable sized
	if(is_variable_sized_type_info(dti1) != is_variable_sized_type_info(dti2))
		return 0;

	// if they both are variable sized, compare their min_size and max_size, else compare their size and is_nullable
	if(!is_variable_sized_type_info(dti1))
	{
		if(dti1->is_nullable != dti2->is_nullable) // they both must be nullable or not
			return 0;
		if(dti1->type == BIT_FIELD)
		{
			if(dti1->bit_field_size != dti2->bit_field_size) // both must have same bit_field_size
				return 0;
		}
		else
		{
			if(dti1->size != dti2->size) // both must have same size
				return 0;
		}
	}
	else
	{
		// both must have same min_size and max_size
		if(dti1->min_size != dti2->min_size)
			return 0;
		if(dti1->max_size != dti2->max_size)
			return 0;
	}

	switch(dti1->type)
	{
		case BIT_FIELD :
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
		case STRING :
		case BLOB :
		default :
			return 1;

		case TUPLE :
		{
			// both must have the same element_count
			if(dti1->element_count != dti2->element_count)
				return 0;

			// and all the element type_infos must be identical
			for(uint32_t i = 0; i < dti1->element_count; i++)
				if(!are_identical_type_info(dti1->containees[i].al.type_info, dti1->containees[i].al.type_info))
					return 0;

			return 1;
		}

		case ARRAY :
		{
			// both must be variable element_count or not
			if(dti1->has_variable_element_count != dti2->has_variable_element_count)
				return 0;

			// if they are not variable element count, then compare their element_count
			if(!dti1->has_variable_element_count)
			{
				if(dti1->element_count != dti2->element_count)
					return 0;
			}

			return are_identical_type_info(dti1->containee, dti1->containee);
		}
	}
}

static void print_tabs(int tabs)
{
	while(tabs)
	{
		printf("\t");
		tabs--;
	}
}

static void print_type_info_recursive(const data_type_info* dti, int tabs)
{
	print_tabs(tabs); printf("type_name : \"%.*s\" (%sfinalized)\n", 64,  dti->type_name, (dti->is_finalized ? "" : "not-"));
	print_tabs(tabs); printf("type : %s\n", types_as_string[dti->type]);
	print_tabs(tabs); printf("is_nullable : %d\n", is_nullable_type_info(dti));
	print_tabs(tabs); printf("is_variable_sized : %d\n", is_variable_sized_type_info(dti));
	print_tabs(tabs); printf("needs_is_valid_bit_in_prefix_bitmap : %d\n", needs_is_valid_bit_in_prefix_bitmap(dti));

	if(dti->type == BIT_FIELD)
	{
		print_tabs(tabs); printf("bit_field_size : %"PRIu32"\n", dti->bit_field_size);
	}
	else if(!is_variable_sized_type_info(dti))
	{
		print_tabs(tabs); printf("size : %"PRIu32"\n", dti->size);
	}
	else
	{
		print_tabs(tabs); printf("min_size : %"PRIu32"\n", dti->min_size);
		print_tabs(tabs); printf("max_size : %"PRIu32"\n", dti->max_size);
	}

	print_tabs(tabs); printf("is_container_type : %d\n", is_container_type_info(dti));

	if(is_container_type_info(dti))
	{
		print_tabs(tabs); printf("has_size_in_prefix : %d\n", has_size_in_its_prefix_for_container_type_info(dti));
		print_tabs(tabs); printf("has_element_count_in_prefix : %d\n", has_element_count_in_its_prefix_for_container_type_info(dti));

		if(dti->has_variable_element_count)
		{
			print_tabs(tabs); printf("element_count : variable\n");
		}
		else
		{
			print_tabs(tabs); printf("element_count : %"PRIu32"\n", dti->element_count);
		}

		if(!dti->has_variable_element_count)
		{
			print_tabs(tabs); printf("prefix_bitmap_size_in_bits : %"PRIu32"\n", dti->prefix_bitmap_size_in_bits);
		}


		if(dti->type == TUPLE)
		{
			print_tabs(tabs + 1); printf("(\n");
			for(uint32_t i = 0; i < dti->element_count; i++)
			{
				const data_type_info* containee_type_info = dti->containees[i].al.type_info;
				const data_position_info* containee_pos_info = dti->containees + i;

				print_tabs(tabs + 2); printf("field_name : %s\n", containee_pos_info->field_name);

				if(containee_type_info->type == BIT_FIELD)
				{
					print_tabs(tabs + 2); printf("bit_offset : %"PRIu32"\n", containee_pos_info->al.bit_offset_in_prefix_bitmap);
				}
				else if(!is_variable_sized_type_info(containee_type_info))
				{
					print_tabs(tabs + 2); printf("byte_offset : %"PRIu32"\n", containee_pos_info->al.byte_offset);
				}
				else
				{
					print_tabs(tabs + 2); printf("byte_offset_to_byte_offset : %"PRIu32"\n", containee_pos_info->al.byte_offset_to_byte_offset);
				}

				if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
				{
					print_tabs(tabs + 2); printf("bit_offset_to_is_valid_bit : %"PRIu32"\n", containee_pos_info->al.bit_offset_to_is_valid_bit);
				}

				print_type_info_recursive(containee_type_info, tabs + 3);
			}
			print_tabs(tabs + 1); printf(")\n");
		}
		else
		{
			print_tabs(tabs + 1); printf("[\n");
			print_type_info_recursive(dti->containee, tabs + 2);
			print_tabs(tabs + 1); printf("]\n");
		}
	}
}

void print_type_info(const data_type_info* dti)
{
	print_type_info_recursive(dti, 0);
}

void print_data_for_data_type_info(const data_type_info* dti, const void* data)
{
	user_value uval;
	get_user_value_for_type_info(&uval, dti, data);
	print_user_value(&uval, dti);
}

uint64_t hash_data_for_type_info(const data_type_info* dti, const void* data, tuple_hasher* th)
{
	if(dti->type == BIT_FIELD)
		return th->hash;

	// if it is not a container hash as is
	if(!is_container_type_info(dti))
		return tuple_hash_bytes(th, data, dti->size);

	for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, data); i++)
	{
		data_positional_info pos_i = INVALID_DATA_POSITIONAL_INFO;

		// if it is a string and the i-th element is a 0
		// containee of a STRING is UINT_NON_NULLABLE[1], i.e. non-nullable hence we do not need to check
		if(dti->type == STRING)
		{
			user_value uval_i;
			get_user_value_to_containee_from_container_CONTAINITY_UNSAFE(&uval_i, dti, data, i, &pos_i);
			if(uval_i.uint_value == 0)
				break;
		}

		hash_containee_in_container(dti, data, i, th, &pos_i);
	}

	return th->hash;
}

uint64_t hash_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, tuple_hasher* th, data_positional_info* containee_pos_info)
{
	if(!is_container_type_info(dti))
		return th->hash;

	// if index is out-of-bounds, treat it as null
	if(index >= get_element_count_for_container_type_info(dti, data))
		return th->hash;

	// if it the index-th containee is null, return 0
	if(is_containee_null_in_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info))
		return th->hash;

	// we now know that the containee must exist
	get_data_positional_info_for_containee_of_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
	const void* child_data = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);

	if(containee_pos_info->type_info == BIT_FIELD)
	{
		for(uint32_t i = 0; i < containee_pos_info->type_info->bit_field_size; i++)
		{
			char bit_data = !!get_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info->bit_offset_in_prefix_bitmap + i);
			tuple_hash_byte(th, bit_data);
		}
		return th->hash;
	}
	else
		return hash_data_for_type_info(containee_pos_info->type_info, child_data, th);
}