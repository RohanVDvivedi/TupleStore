#include<data_type_info.h>
#include<page_layout_util.h>

#include<serial_int.h>
#include<large_uints.h>

#include<bitmap.h>

char type_as_string[][16] = {
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

int is_nullable_type_info(const data_type_info* dti)
{
	return is_variable_sized_type_info(dti) || dti->is_nullable;
}

int needs_is_valid_bit_in_prefix_bitmap(const data_type_info* dti)
{
	return (!is_variable_sized_type_info(dti)) && dti->is_nullable;
}

int is_variable_sized_type_info(const data_type_info* dti)
{
	return dti->is_variable_sized;
}

uint32_t get_size_for_type_info(const data_type_info* dti, const void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
		return dti->size;

	// variable sized element either has its size in prefix or its element count in the prefix or both
	if(has_size_in_its_prefix_for_container_type_info(dti))
		return read_value_from_page(data + get_offset_to_prefix_size_for_container_type_info(dti), get_bytes_required_for_prefix_size_for_container_type_info(dti));

	uint32_t element_count = read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), get_bytes_required_for_prefix_element_count_for_container_type_info(dti));

	// now we know for sure that this is variable sized container, but without size in its prefix
	// so this must be a container precisely : variable sized string, variable sized blob or array of variable element count but of fixed length type
	// all in all we know the element_count and that each element is fixed sized element
	
	if(dti->containee->type == BIT_FIELD)
		return get_value_size_on_page(dti->max_size) // it has element_count in its prefix but not its size
		 + bitmap_size_in_bytes(element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size));
	else
		return get_value_size_on_page(dti->max_size) // it has element_count in its prefix but not its size
		 + bitmap_size_in_bytes(element_count * needs_is_valid_bit_in_prefix_bitmap(dti->containee))
		 + (element_count * dti->containee->size);
}

int is_container_type_info(const data_type_info* dti)
{
	return dti->type == STRING || dti->type == BLOB || dti->type == TUPLE || dti->type == ARRAY;
}

// check if variable element_count
int has_variable_element_count_for_container_type_info(const data_type_info* dti)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// not possible for fixed sized element
	if(!is_variable_sized_type_info(dti))
		return 0;

	// tuples are by default fixed element_count, yet they could be variable sized
	if(dti->type == TUPLE)
		return 0;

	// a variable sized string or blob is always variable element count container
	if(dti->type == STRING || dti->type == BLOB)
		return 1;

	// this leaves us with variable sized strings, blobs and arrays with either variable element count or containing variable sized elements
	return dti->has_variable_element_count;
}

uint32_t get_element_count_for_container_type_info(const data_type_info* dti, const void* data)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// if not variable element_count then read from the type_info
	if(!has_variable_element_count_for_container_type_info(dti))
		return dti->element_count;

	// else read from the data
	return read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), get_bytes_required_for_prefix_element_count_for_container_type_info(dti));
}

int has_size_in_its_prefix_for_container_type_info(const data_type_info* dti)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// not possible for fixed sized element
	if(!is_variable_sized_type_info(dti))
		return 0;

	// size must be stored for variable sized tuples
	if(dti->type == TUPLE)
		return 1;

	// not required for a variable sized string or blob
	// we are already storing element_count for them
	if(dti->type == STRING || dti->type == BLOB)
		return 0;

	// for an array we need to store the total size only if the individual elements that it contains is variable sized
	return is_variable_sized_type_info(dti->containee);
}

int has_element_count_in_its_prefix_for_container_type_info(const data_type_info* dti)
{
	return has_variable_element_count_for_container_type_info(dti);
}

uint32_t get_prefix_bitmap_size_in_bits_for_container_type_info(const data_type_info* dti, const void* data)
{
	// no prefix bitmap for non container type data_types
	if(!is_container_type_info(dti))
		return 0;

	// for tuple used the cached value
	if(dti->type == TUPLE)
		return dti->prefix_bitmap_size_in_bits;

	// no prefix bitmap necessary for STRING and BLOB
	if(dti->type == STRING || dti->type == BLOB)
		return 0;

	// this must now be an array

	if(dti->containee->type == BIT_FIELD)
		return get_element_count_for_container_type_info(dti, data) * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size);
	else
		return get_element_count_for_container_type_info(dti, data) * needs_is_valid_bit_in_prefix_bitmap(dti->containee);
}

data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return NULL;

	// same thing, if the index is out of bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return NULL;

	if(dti->type == TUPLE)
		return dti->containees[index].type_info;

	if(dti->type == STRING || dti->type == BLOB)
		return UINT_1_NON_NULLABLE; // this must be the containee here, so why not return the default

	// else it has to be an array
	return dti->containee;
}

data_position_info get_data_position_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return (data_position_info){};

	// same thing, if the index is out of bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return (data_position_info){};

	// for a tuple return a precomputed value
	if(dti->type == TUPLE)
		return dti->containees[index];

	// case statement for strings, blobs and arrays

	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);
	uint32_t first_element_offset = prefix_bitmap_offset + get_prefix_bitmap_size_for_container_type_info(dti, data);

	data_type_info* containee_type_info = get_data_type_info_for_containee_of_container(dti, data, index);

	if(containee_type_info->type == BIT_FIELD)
	{
		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
		{
			return (data_position_info){
				.bit_offset_in_prefix_bitmap = ((1 + containee_type_info->bit_field_size) * index) + 1,
				.bit_offset_to_is_valid_bit = ((1 + containee_type_info->bit_field_size) * index),
				.type_info = containee_type_info,
			};
		}
		else
		{
			return (data_position_info){
				.bit_offset_in_prefix_bitmap = containee_type_info->bit_field_size * index,
				.bit_offset_to_is_valid_bit = 0, // will be unused
				.type_info = containee_type_info,
			};
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		return (data_position_info){
			.byte_offset = first_element_offset + containee_type_info->size * index,
			.bit_offset_to_is_valid_bit = index * needs_is_valid_bit_in_prefix_bitmap(containee_type_info), // gets set to 0, if it won't need a is_valid bit in prefix_bitmap
			.type_info = containee_type_info,
		};
	}
	else
	{
		return (data_position_info){
			.byte_offset_to_byte_offset = first_element_offset + get_value_size_on_page(dti->max_size) * index,
			.type_info = containee_type_info,
		};
	}
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
			dti->containee = UINT_1_NON_NULLABLE;
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
				data_type_info* containee_type_info = dti->containees[i].type_info;
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
					containee_pos_info->bit_offset_to_is_valid_bit = dti->prefix_bitmap_size_in_bits;
					dti->prefix_bitmap_size_in_bits += 1;
				}

				// assign bit offsets in prefix_bitmap, to the bit_fields
				if(containee_type_info->type == BIT_FIELD)
				{
					// assign bit_offset to bit_field and increment the prefix_bitmap_size_in_bits
					containee_pos_info->bit_offset_in_prefix_bitmap = dti->prefix_bitmap_size_in_bits;
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
				data_type_info* containee_type_info = dti->containees[i].type_info;
				data_position_info* containee_pos_info = dti->containees + i;

				// all the work needed for BIT_FIELD is already done, hence we can skip it
				if(containee_type_info->type == BIT_FIELD)
					continue;

				if(is_variable_sized_type_info(containee_type_info))
				{
					containee_pos_info->byte_offset_to_byte_offset = dti->min_size;
					dti->min_size += get_value_size_on_page(dti->max_size);
				}
				else
				{
					containee_pos_info->byte_offset = dti->size;
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

#include<stdio.h>

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
	print_tabs(tabs); printf("type_name : %s (%sfinalized)\n", dti->type_name, (dti->is_finalized ? "" : "not-"));
	print_tabs(tabs); printf("type : %s\n", type_as_string[dti->type]);
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
				const data_type_info* containee_type_info = dti->containees[i].type_info;
				const data_position_info* containee_pos_info = dti->containees + i;

				print_tabs(tabs + 2); printf("field_name : %s\n", containee_pos_info->field_name);

				if(containee_type_info->type == BIT_FIELD)
				{
					print_tabs(tabs + 2); printf("bit_offset : %"PRIu32"\n", containee_pos_info->bit_offset_in_prefix_bitmap);
				}
				else if(!is_variable_sized_type_info(containee_type_info))
				{
					print_tabs(tabs + 2); printf("byte_offset : %"PRIu32"\n", containee_pos_info->byte_offset);
				}
				else
				{
					print_tabs(tabs + 2); printf("byte_offset_to_byte_offset : %"PRIu32"\n", containee_pos_info->byte_offset_to_byte_offset);
				}

				if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
				{
					print_tabs(tabs + 2); printf("bit_offset_to_is_valid_bit : %"PRIu32"\n", containee_pos_info->bit_offset_to_is_valid_bit);
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

int is_containee_null_in_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 1;

	// make sure that index is withint bounds, else it is said to be NULL
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 1;

	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	// a non-nullable element can never be null
	if(!is_nullable_type_info(containee_pos_info.type_info))
		return 0;

	if(!is_variable_sized_type_info(containee_pos_info.type_info))
	{
		// must have a bit in is_valid bitmap
		return get_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info.bit_offset_to_is_valid_bit) == 0;
	}
	else
	{
		// variable size element is NULL if the byte_offset in the tuple of the containee is 0
		return read_value_from_page(data + containee_pos_info.byte_offset_to_byte_offset, dti->max_size) == 0;
	}
}

const void* get_containee_from_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return NULL;

	// make sure that index is withint bounds, else it is said to be NULL
	if(index >= get_element_count_for_container_type_info(dti, data))
		return NULL;

	// if it is already null return NULL
	if(is_containee_null_in_container(dti, data, index))
		return NULL;

	// TODO
}