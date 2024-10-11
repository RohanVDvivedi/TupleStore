#include<data_type_info.h>
#include<page_layout_util.h>

#include<serial_int.h>
#include<large_uints.h>
#include<float_accesses.h>

#include<bitmap.h>

#include<string.h>
#include<stdio.h>

char types_as_string[][16] = {
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
	{
		uint32_t size = read_value_from_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size);
		return (size == 0) ? dti->max_size : size; // a variable sized element is never 0 sized (because it is storing size in prefix), it is probably max_size
	}

	uint32_t element_count = read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size);

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

int overwrite_size_for_container_type_info_with_size_in_prefix(const data_type_info* dti, void* data, uint32_t new_size)
{
	if(!has_size_in_its_prefix_for_container_type_info(dti))
		return 0;

	if(new_size > dti->max_size)
		return 0;

	write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);

	return 1;
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
	return read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size);
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

data_type_info* get_data_type_info_for_containee_of_container_without_data(const data_type_info* dti, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return NULL;

	// for fixed element count containers make sure that index is without bounds
	if(!has_variable_element_count_for_container_type_info(dti) && index >= dti->element_count)
		return NULL;

	// proceed as now the index is probably within bounds

	if(dti->type == TUPLE)
		return dti->containees[index].type_info;

	if(dti->type == STRING || dti->type == BLOB)
		return UINT_NON_NULLABLE[1]; // this must be the containee here, so why not return the default

	// else it has to be an array
	return dti->containee;
}

data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return NULL;

	// same thing, if the index is out of bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return NULL;

	// index is now surely within bounds

	if(dti->type == TUPLE)
		return dti->containees[index].type_info;

	if(dti->type == STRING || dti->type == BLOB)
		return UINT_NON_NULLABLE[1]; // this must be the containee here, so why not return the default

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

	// index is now surely within bounds

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
				if(!are_identical_type_info(dti1->containees[i].type_info, dti1->containees[i].type_info))
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

const void* get_pointer_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index)
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

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	if(containee_pos_info.type_info->type == BIT_FIELD)
		return data + get_offset_to_prefix_bitmap_for_container_type_info(dti); // returning the pointer to the completee bitmap if the element is a bitfield
	else if(!is_variable_sized_type_info(containee_pos_info.type_info))
		return data + containee_pos_info.byte_offset;
	else
		return data + read_value_from_page(data + containee_pos_info.byte_offset_to_byte_offset, dti->max_size);
}

uint32_t get_size_of_containee_from_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// must be a container type info to call this function
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is withint bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	if(containee_pos_info.type_info->type == BIT_FIELD)
		return 0;
	else if(!is_variable_sized_type_info(containee_pos_info.type_info))
		return dti->size;
	else
	{
		const void* containee = get_pointer_to_containee_from_container(dti, data, index);
		if(containee == NULL) // a NULL varibale sized element is said to be not containing any space
			return 0;
		else
			return get_size_for_type_info(containee_pos_info.type_info, containee);
	}
}

const user_value get_user_value_for_type_info(const data_type_info* dti, const void* data)
{
	if(data == NULL)
		return (*NULL_USER_VALUE);

	user_value uval = {};
	switch(dti->type)
	{
		case BIT_FIELD :
		{
			return (*NULL_USER_VALUE);
		}
		case UINT :
		{
			uval.uint_value = deserialize_uint64(data, dti->size);
			break;
		}
		case INT :
		{
			uval.int_value = deserialize_int64(data, dti->size);
			break;
		}
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				uval.float_value = deserialize_float(data);
			else if(dti->size == sizeof(double))
				uval.double_value = deserialize_double(data);
			else if(dti->size == sizeof(long double))
				uval.long_double_value = deserialize_long_double(data);
		}
		case LARGE_UINT :
		{
			uval.large_uint_value = deserialize_uint256(data, dti->size);
			break;
		}
		case STRING :
		{
			// grab pointer to the first byte, and the element_count of the string, since it is inherently an array of non-nullable fixed length elements, they are placed sequential after the first byte
			uval.string_value = data + get_data_position_info_for_containee_of_container(dti, data, 0).byte_offset;
			uval.string_size = get_element_count_for_container_type_info(dti, data);
			// this string could be null terminated
			uval.string_size = strnlen(uval.string_value, uval.string_size);
			break;
		}
		case BLOB :
		{
			// grab pointer to the first byte, and the element_count of the blob, since it is inherently an array of non-nullable fixed length elements, they are placed sequential after the first byte
			uval.blob_value = data + get_data_position_info_for_containee_of_container(dti, data, 0).byte_offset;
			uval.blob_size = get_element_count_for_container_type_info(dti, data);
			break;
		}
		case TUPLE :
		{
			uval.tuple_value = data;
			break;
		}
		case ARRAY :
		{
			uval.array_value = data;
			break;
		}
	}

	return uval;
}

const user_value get_user_value_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// dti has to be a container type, else we can not index it and so we return OUT_OF_BOUNDS_USER_VALUE
	if(!is_container_type_info(dti))
		return (*OUT_OF_BOUNDS_USER_VALUE);

	// make sure that index is within bounds, else it is said to be OUT_OF_BOUNDS_USER_VALUE
	if(index >= get_element_count_for_container_type_info(dti, data))
		return (*OUT_OF_BOUNDS_USER_VALUE);

	// if it is already null return NULL_USER_VALUE
	if(is_containee_null_in_container(dti, data, index))
		return (*NULL_USER_VALUE);

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);
	const void* containee = get_pointer_to_containee_from_container(dti, data, index);

	user_value uval = {};
	switch(containee_pos_info.type_info->type)
	{
		case BIT_FIELD :
		{
			uval.bit_field_value = get_bits(containee, containee_pos_info.bit_offset_in_prefix_bitmap, containee_pos_info.bit_offset_in_prefix_bitmap + containee_pos_info.type_info->bit_field_size - 1);
			return uval;
		}
		default :
			return get_user_value_for_type_info(containee_pos_info.type_info, containee);
	}
}

int is_variable_sized_containee_at_end_of_container(const data_type_info* dti, void* data, uint32_t index)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is withint bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if it is already null, fail
	if(is_containee_null_in_container(dti, data, index))
		return 0;

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	// if this element is not variable sized then fail
	if(!is_variable_sized_type_info(containee_pos_info.type_info))
		return 0;

	void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index);
	uint32_t containee_byte_offset = containee - data;
	uint32_t containee_size = get_size_for_type_info(containee_pos_info.type_info, containee);
	uint32_t container_size = get_size_for_type_info(dti, data);

	// the containee is at the end of the container
	if(containee_byte_offset + containee_size == container_size)
		return 1;

	return 0;
}

int move_variable_sized_containee_to_end_of_container(const data_type_info* dti, void* data, uint32_t index)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is withint bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if it is already null, fail
	if(is_containee_null_in_container(dti, data, index))
		return 0;

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	// if this element is not variable sized then fail
	if(!is_variable_sized_type_info(containee_pos_info.type_info))
		return 0;

	// if the index-th element is already at the end then fail
	if(is_variable_sized_containee_at_end_of_container(dti, data, index))
		return 1;

	void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index);
	uint32_t containee_byte_offset = containee - data;
	uint32_t containee_size = get_size_for_type_info(containee_pos_info.type_info, containee);
	uint32_t container_size = get_size_for_type_info(dti, data);

	// perform left rotation to psuh the containee at index to the end of the container
	memory_left_rotate(containee, container_size - containee_byte_offset, containee_size);

	for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, data); i++)
	{
		data_position_info pos_info_i = get_data_position_info_for_containee_of_container(dti, data, i);

		// the offsets have to be adjusted but not for the index-th element and not for the fixed sized elements
		if(i == index || !is_variable_sized_type_info(pos_info_i.type_info))
			continue;

		// move all offsets to elements that came after the containee_byte_offset front by the containee_size
		uint32_t byte_offset_i = read_value_from_page(data + pos_info_i.byte_offset_to_byte_offset, dti->max_size);

		if(byte_offset_i > containee_byte_offset)
			write_value_to_page(data + pos_info_i.byte_offset_to_byte_offset, dti->max_size, byte_offset_i - containee_size);
	}

	// finally update the offset of the index-th element
	write_value_to_page(data + containee_pos_info.byte_offset_to_byte_offset, dti->max_size, container_size - containee_size);
	return 1;
}

uint32_t initialize_minimal_data_for_type_info(const data_type_info* dti, void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
	{
		memory_set(data, 0, dti->size);
		return dti->size;
	}
	else
	{
		memory_set(data, 0, dti->min_size);
		// if it has size set it to min_size, element_count if exists on the data is set to 0 by the above statement
		if(has_size_in_its_prefix_for_container_type_info(dti))
			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, dti->min_size);
		return dti->min_size;
	}
}

static int are_zeroes(const char* data, uint32_t data_size)
{
	for(uint32_t i = 0; i < data_size; i++)
		if(data[i] != 0)
			return 0;
	return 1;
}

int is_minimal_data_for_type_info(const data_type_info* dti, const void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
		return are_zeroes(data, dti->size); // if not variable sized the completed content must be 0
	else
	{
		uint32_t data_size = get_size_for_type_info(dti, data);
		if(data_size != dti->min_size) // if variable sized the size must equal min_size
			return 0;

		// and the remaining content must be all zeroes
		if(has_size_in_its_prefix_for_container_type_info(dti))
			return are_zeroes(data, get_offset_to_prefix_size_for_container_type_info(dti)) &&
					are_zeroes(data + get_offset_to_prefix_size_for_container_type_info(dti) + get_value_size_on_page(dti->max_size),
								data_size - (get_offset_to_prefix_size_for_container_type_info(dti) + get_value_size_on_page(dti->max_size)));
		else
			return are_zeroes(data, data_size);
	}
}

int set_containee_to_NULL_in_container(const data_type_info* dti, void* data, uint32_t index)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if it is already null, succeed
	if(is_containee_null_in_container(dti, data, index))
		return 1;

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	// a non-nullable element can never be null
	if(!is_nullable_type_info(containee_pos_info.type_info))
		return 0;

	if(needs_is_valid_bit_in_prefix_bitmap(containee_pos_info.type_info)) // must be a BIT_FIELD or a fixed sized field
		reset_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info.bit_offset_to_is_valid_bit);
	else // else it is a variable sized field
	{
		uint32_t container_size = get_size_for_type_info(dti, data);
		uint32_t containee_size = get_size_for_type_info(containee_pos_info.type_info, get_pointer_to_containee_from_container(dti, data, index));

		// move containee to the end of the container
		move_variable_sized_containee_to_end_of_container(dti, data, index);

		// set containee offset to 0
		write_value_to_page(data + containee_pos_info.byte_offset_to_byte_offset, dti->max_size, 0);

		// if it has size in its prefix deduct containee_size from it
		if(has_size_in_its_prefix_for_container_type_info(dti))
			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, container_size - containee_size);
	}

	return 1;
}

int can_set_user_value_for_type_info(const data_type_info* dti, const void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval)
{
	if(is_user_value_NULL(uval) || is_user_value_OUT_OF_BOUNDS(uval))
		return 0;

	if(dti->type == BIT_FIELD)
		return 0;

	// if it is fixed sized, then no need to check for max_size_increment
	if(!is_variable_sized_type_info(dti))
		return 1;

	switch(dti->type)
	{
		case STRING :
		{
			user_value uval_t = *uval;

			// limit the string length
			uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);

			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval_t.string_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case BLOB :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval->blob_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case TUPLE :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->tuple_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case ARRAY :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->array_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

int set_user_value_for_type_info(const data_type_info* dti, void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval)
{
	if(is_user_value_NULL(uval) || is_user_value_OUT_OF_BOUNDS(uval))
		return 0;

	if(dti->type == BIT_FIELD)
		return 0;

	// if it is fixed sized, then no need to check for max_size_increment
	if(!is_variable_sized_type_info(dti))
	{
		switch(dti->type)
		{
			case UINT :
			{
				serialize_uint64(data, dti->size, uval->uint_value);
				return 1;
			}
			case INT :
			{
				serialize_int64(data, dti->size, uval->int_value);
				return 1;
			}
			case FLOAT :
			{
				if(dti->size == sizeof(float))
					serialize_float(data, uval->float_value);
				else if(dti->size == sizeof(double))
					serialize_double(data, uval->double_value);
				else if(dti->size == sizeof(long double))
					serialize_long_double(data, uval->long_double_value);
				return 1;
			}
			case LARGE_UINT :
			{
				serialize_uint256(data, dti->size, uval->large_uint_value);
				return 1;
			}
			case STRING :
			{
				user_value uval_t = *uval;

				// limit the string length
				uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);
				uval_t.string_size = min(uval_t.string_size, dti->size);

				// copy contents to data
				memory_move(data, uval_t.string_value, uval_t.string_size);
				// padd remaining bytes to 0
				if(uval_t.string_size < dti->size)
					memory_set(data + uval_t.string_size, 0, dti->size - uval_t.string_size);
				return 1;
			}
			case BLOB :
			{
				user_value uval_t = *uval;

				uval_t.blob_size = min(uval_t.blob_size, dti->size);
				// copy contents to data
				memory_move(data, uval_t.blob_value, uval_t.blob_size);
				return 1;
			}
			case TUPLE :
			{
				// copy contents to data
				if(uval != EMPTY_USER_VALUE) // if user provided this pointer, then the tuple_value is NULL, hence we need to initialize a minimal tuple at this position
					memory_move(data, uval->tuple_value, dti->size);
				else
					initialize_minimal_data_for_type_info(dti, data);
				return 1;
			}
			case ARRAY :
			{
				// copy contents to data
				if(uval != EMPTY_USER_VALUE) // if user provided this pointer, then the array_value is NULL, hence we need to initialize a minimal tuple at this position
					memory_move(data, uval->array_value, dti->size);
				else
					initialize_minimal_data_for_type_info(dti, data);
				return 1;
			}
			default :
			{
				return 0;
			}
		}
	}

	// now we know for sure that it is a variable sized type
	// it can be only one of these composite types
	switch(dti->type)
	{
		case STRING :
		{
			user_value uval_t = *uval;

			// limit the string length
			uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);

			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval_t.string_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// write element count and copy contents to data
			write_value_to_page(data, dti->max_size, uval_t.string_size);
			memory_move(data + get_value_size_on_page(dti->max_size), uval_t.string_value, uval_t.string_size);
			return 1;
		}
		case BLOB :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval->blob_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// write element count and copy contents to data
			write_value_to_page(data, dti->max_size, uval->blob_size);
			memory_move(data + get_value_size_on_page(dti->max_size), uval->blob_value, uval->blob_size);
			return 1;
		}
		case TUPLE :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->tuple_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// copy contents to data
			if(uval != EMPTY_USER_VALUE)
				memory_move(data, uval->tuple_value, dti->size);
			else
				initialize_minimal_data_for_type_info(dti, data);
			return 1;
		}
		case ARRAY :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->array_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// copy contents to data
			if(uval != EMPTY_USER_VALUE)
				memory_move(data, uval->array_value, dti->size);
			else
				initialize_minimal_data_for_type_info(dti, data);
			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

int can_set_user_value_to_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// an out of bounds containee is never accessible
	if(is_user_value_OUT_OF_BOUNDS(uval))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	// if uval is NULL, set it to NULL
	// this will never increment the size requirement, hence no checks required
	if(is_user_value_NULL(uval))
		return is_nullable_type_info(containee_pos_info.type_info);

	// there will already be enough space for the fixed sized containee
	if(!is_variable_sized_type_info(containee_pos_info.type_info))
		return 1;

	uint32_t old_container_size = get_size_for_type_info(dti, data);
	max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - old_container_size);

	const void* containee = get_pointer_to_containee_from_container(dti, data, index);
	int is_old_containee_offset_valid = (containee != NULL);
	if(containee == NULL)
		containee = data + old_container_size;

	return can_set_user_value_for_type_info(containee_pos_info.type_info, containee, is_old_containee_offset_valid, max_size_increment_allowed, uval);
}

int set_user_value_to_containee_in_container(const data_type_info* dti, void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// an out of bounds containee is never accessible
	if(is_user_value_OUT_OF_BOUNDS(uval))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if uval is NULL, set it to NULL
	// this will never increment the size requirement, hence no checks required
	if(is_user_value_NULL(uval))
		return set_containee_to_NULL_in_container(dti, data, index);

	// now we are sure that uval is not NULL

	// fetch information about containee
	data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

	if(!is_variable_sized_type_info(containee_pos_info.type_info))
	{
		// if it has is_valid_bit then set it
		if(needs_is_valid_bit_in_prefix_bitmap(containee_pos_info.type_info)) // must be a BIT_FIELD or a fixed sized field
			set_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info.bit_offset_to_is_valid_bit);

		if(containee_pos_info.type_info->type == BIT_FIELD)
		{
			set_bits(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info.bit_offset_in_prefix_bitmap, containee_pos_info.bit_offset_in_prefix_bitmap + containee_pos_info.type_info->bit_field_size - 1, uval->bit_field_value);
			return 1;
		}
		else
		{
			void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index);
			return set_user_value_for_type_info(containee_pos_info.type_info, containee, 1 /* this attribute is NO-OP here */, max_size_increment_allowed, uval);
		}
	}
	else
	{
		// first thing we need to do is send the concerned containee to the end of the container
		move_variable_sized_containee_to_end_of_container(dti, data, index);

		// moving the element changes its position in the container, hence the recalculation
		containee_pos_info = get_data_position_info_for_containee_of_container(dti, data, index);

		uint32_t old_container_size = get_size_for_type_info(dti, data);
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - old_container_size);

		void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index);
		uint32_t old_containee_size = 0;
		int is_old_containee_offset_valid = (containee != NULL);

		// if the containee is NULL, then old_containee_size = 0
		// and the position to insert the containee is at the end of the container
		if(containee == NULL)
			containee = data + old_container_size;
		else
			old_containee_size = get_size_for_type_info(containee_pos_info.type_info, containee);

		int result = set_user_value_for_type_info(containee_pos_info.type_info, containee, is_old_containee_offset_valid, max_size_increment_allowed, uval);

		// if result was a success, and the old_containee_offset was in-valid i.e 0, then set it
		if(result && !is_old_containee_offset_valid)
			write_value_to_page(data + containee_pos_info.byte_offset_to_byte_offset, dti->max_size, old_container_size);

		// if the container has size in prefix then update it
		if(result && has_size_in_its_prefix_for_container_type_info(dti))
		{
			uint32_t new_containee_size = get_size_for_type_info(containee_pos_info.type_info, containee);

			uint32_t new_container_size = old_container_size - old_containee_size + new_containee_size;

			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_container_size);
		}

		return result;
	}
}

int can_expand_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// it's element_count must be variable
	if(!has_variable_element_count_for_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count], else fail
	if(index > get_element_count_for_container_type_info(dti, data))
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count + slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;
	}
	else
	{
		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		new_size = old_size + (byte_offset_size * slots);
	}

	if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
		return 0;

	return 1;
}

int expand_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// it's element_count must be variable
	if(!has_variable_element_count_for_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count], else fail
	if(index > get_element_count_for_container_type_info(dti, data))
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count + slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new bits
		for(uint32_t i = 0; i < old_element_count - index; i++)
		{
			uint32_t copy_from_bit = (old_element_count - 1 - i) * prefix_bits_necessary_for_1_containee;
			uint32_t copy_to_bit = (new_element_count - 1 - i) * prefix_bits_necessary_for_1_containee;
			uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit + prefix_bits_necessary_for_1_containee - 1);
			set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit + prefix_bits_necessary_for_1_containee - 1, t);
		}

		// make new slots 0
		for(uint32_t i = 0; i < slots; i++)
		{
			uint32_t to_zero_bit = (index + i) * prefix_bits_necessary_for_1_containee;
			set_bits(data + prefix_bitmap_offset, to_zero_bit, to_zero_bit + prefix_bits_necessary_for_1_containee - 1, 0);
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_old_size = bitmap_size_in_bytes(old_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new bits if required
		if(prefix_bitmap_old_size != prefix_bitmap_new_size)
		{
			uint32_t copy_from = prefix_bitmap_offset + prefix_bitmap_old_size;
			uint32_t copy_to = prefix_bitmap_offset + prefix_bitmap_new_size;
			memory_move(data + copy_to, data + copy_from, old_size - copy_from);
		}

		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info)) // if these element have is_valid bits in prefix, then move them accordingly or 0 them if they are new
		{
			for(uint32_t i = 0; i < old_element_count - index; i++)
			{
				uint32_t copy_from_bit = old_element_count - 1 - i;
				uint32_t copy_to_bit = new_element_count - 1 - i;
				uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit);
				set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit, t);
			}

			// make new slots 0
			for(uint32_t i = 0; i < slots; i++)
			{
				uint32_t to_zero_bit = (index + i);
				reset_bit(data + prefix_bitmap_offset, to_zero_bit);
			}
		}

		// calculate the offset to the array of containees
		uint32_t offset_to_first_element = prefix_bitmap_offset + prefix_bitmap_new_size;

		// make room from new slots for the fixed sized containees
		memory_move(data + offset_to_first_element + ((index + slots) * byte_size), data + offset_to_first_element + (index * byte_size), (old_element_count - index) * byte_size);

		// zero out the new slots
		memory_set(data + offset_to_first_element + (index * byte_size), 0, slots * byte_size);
	}
	else
	{
		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		// calculate new size and check for size increments
		new_size = old_size + (byte_offset_size * slots);
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new slots
		uint32_t copy_from = prefix_bitmap_offset + byte_offset_size * index;
		uint32_t copy_to = prefix_bitmap_offset + byte_offset_size * (index + slots);
		memory_move(data + copy_to, data + copy_from, old_size - copy_from);

		// zero out the new slots
		memory_set(data + copy_from, 0, copy_to - copy_from);

		// since all the varibale length data is moved back by (copy_to - copy_from) bytes, we need to update their offsets
		for(uint32_t i = 0; i < new_element_count; i++)
		{
			uint32_t offset = read_value_from_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size);
			if(offset != 0)
				write_value_to_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size, offset + (copy_to - copy_from));
		}
	}

	// update the size and element count in the prefix
	if(has_size_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);
	if(has_element_count_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size, new_element_count);

	return 1;
}

int can_discard_from_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// it's element_count must be variable
	if(!has_variable_element_count_for_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count-1], else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// must have atleast slots number of slots after first index number of slots
	if(get_element_count_for_container_type_info(dti, data) - index < slots)
		return 0;

	// else it will surely succeed
	return 1;
}

int discard_from_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// it's element_count must be variable
	if(!has_variable_element_count_for_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count-1], else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// must have atleast slots number of slots after first index number of slots
	if(get_element_count_for_container_type_info(dti, data) - index < slots)
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count - slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		// calculate new size
		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);

		// move succeeding bits to prior location
		for(uint32_t i = 0; i < old_element_count - (index + slots); i++)
		{
			uint32_t copy_from_bit = ((index + slots) + i) * prefix_bits_necessary_for_1_containee;
			uint32_t copy_to_bit = (index + i) * prefix_bits_necessary_for_1_containee;
			uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit + prefix_bits_necessary_for_1_containee - 1);
			set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit + prefix_bits_necessary_for_1_containee - 1, t);
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_old_size = bitmap_size_in_bytes(old_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;

		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info)) // if these element have is_valid bits in prefix, move them back
		{
			for(uint32_t i = 0; i < old_element_count - (index + slots); i++)
			{
				uint32_t copy_from_bit = ((index + slots) + i);
				uint32_t copy_to_bit = (index + i);
				uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit);
				set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit, t);
			}
		}

		// move data containing containees, if the bitmap shrunk
		if(prefix_bitmap_old_size != prefix_bitmap_new_size)
		{
			uint32_t copy_from = prefix_bitmap_offset + prefix_bitmap_old_size;
			uint32_t copy_to = prefix_bitmap_offset + prefix_bitmap_new_size;
			memory_move(data + copy_to, data + copy_from, old_size - copy_from);
		}

		// calculate the offset to the array of containees
		uint32_t offset_to_first_element = prefix_bitmap_offset + prefix_bitmap_new_size;

		// discard the concerning slots
		memory_move(data + offset_to_first_element + (index * byte_size), data + offset_to_first_element + ((index + slots) * byte_size), (old_element_count - (index + slots)) * byte_size);
	}
	else
	{
		// first make the concerning slots NULL
		for(uint32_t i = 0; i < slots; i++)
			set_containee_to_NULL_in_container(dti, data, index + i);

		// update the old size that we have cached
		old_size = get_size_for_type_info(dti, data);

		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		// calculate new size
		new_size = old_size - (byte_offset_size * slots);

		// make room for new slots
		uint32_t copy_from = prefix_bitmap_offset + byte_offset_size * (index + slots);
		uint32_t copy_to = prefix_bitmap_offset + byte_offset_size * index;
		memory_move(data + copy_to, data + copy_from, old_size - copy_from);

		// since all the varibale length data is moved back by (copy_from - copy_to) bytes, we need to update their offsets
		for(uint32_t i = 0; i < new_element_count; i++)
		{
			uint32_t offset = read_value_from_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size);
			if(offset != 0)
				write_value_to_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size, offset - (copy_from - copy_to));
		}
	}

	// update the size and element count in the prefix
	if(has_size_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);
	if(has_element_count_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size, new_element_count);

	return 1;
}

void print_data_for_data_type_info(const data_type_info* dti, const void* data)
{
	const user_value uval = get_user_value_for_type_info(dti, data);
	print_user_value(&uval, dti);
}

uint64_t hash_data_for_type_info(const data_type_info* dti, const void* data, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	if(dti->type == BIT_FIELD)
		return 0;

	// if it is not a container hash as is
	if(!is_container_type_info(dti))
		return hash_func(data, dti->size);

	uint32_t hash = 0;
	for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, data); i++)
	{
		// if it is a string and the i-th element is a 0
		// containee of a STRING is UINT_NON_NULLABLE[1], i.e. non-nullable hence we do not need to check
		if(dti->type == STRING && get_user_value_to_containee_from_container(dti, data, i).uint_value == 0)
			break;

		hash ^= hash_containee_in_container(dti, data, i, hash_func);
	}

	return hash;
}

uint64_t hash_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	if(!is_container_type_info(dti))
		return 0;

	// if index is out-of-bounds, treat it as null
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if it the index-th containee is null, return 0
	if(is_containee_null_in_container(dti, data, index))
		return 0;

	// we now know that the containee must exist
	data_position_info child_pos = get_data_position_info_for_containee_of_container(dti, data, index);
	const void* child_data = get_pointer_to_containee_from_container(dti, data, index);

	if(child_pos.type_info == BIT_FIELD)
	{
		uint64_t hash = 0;
		for(uint32_t i = 0; i < child_pos.type_info->bit_field_size; i++)
		{
			char bit_data = !!get_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), child_pos.bit_offset_in_prefix_bitmap + i);
			hash ^= hash_func(&bit_data, 1);
		}
		return hash;
	}
	else
		return hash_data_for_type_info(child_pos.type_info, child_data, hash_func);
}