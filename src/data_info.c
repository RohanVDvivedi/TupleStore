#include<data_info.h>

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
	if(!is_variable_sized_size_info(dti))
		return dti->.size;

	// variable sized element either has its size in prefix or its element count in the prefix
	if(has_size_in_its_prefix_for_container_type_info(dti))
		return read_value_from_page(data + get_offset_to_prefix_size_for_container_type_info(dti), get_bytes_required_for_prefix_size_for_container_type_info(dti));

	uint32_t element_count = read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), get_bytes_required_for_prefix_element_count_for_container_type_info(dti));

	// now we know for sure that this is variable sized container, but without size intis prefix
	// so this must be a variable element count array of fixed length elements
	// TODO
}

int is_container_type_info(const data_size_info* dti)
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

// get element_count
uint32_t get_element_count_for_container_type_info(const data_type_info* dti, const void* data);

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

#define get_offset_to_prefix_size_for_container_type_info(dti)						(0)
#define get_bytes_required_for_prefix_size_for_container_type_info(dti) 			(get_value_size_on_page((dti)->size_info.max_size) * has_size_in_its_prefix_for_container_type_info(dti))

#define get_offset_to_prefix_element_count_for_container_type_info(dti)				(get_offset_to_prefix_size_for_container_type_info(dti) + get_bytes_required_for_prefix_size_for_container_type_info(dti))
#define get_bytes_required_for_prefix_element_count_for_container_type_info(dti) 	(get_value_size_on_page((dti)->size_info.max_size) * has_element_count_in_its_prefix_for_container_type_info(dti))

// logically equivalent to = bytes_required_for_prefix_size + bytes_required_for_prefix_element_count
#define get_offset_to_prefix_bitmap_for_container_type_info(dti)					(get_offset_to_prefix_element_count_for_container_type_info(dti) + get_bytes_required_for_prefix_element_count_for_container_type_info(dti))

// valid only for a container type info, returns the number of bits required in the prefix for the container type info
// for a tuple or a fixed element count array of fixed length elements it is equal to dti->prefix_bitmap_size_in_bits
// for an array of variable length elements it is always 0
// for a variable element count array of fixed length elements, it will be equal to element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + (dti->containee.type == BIT_FIELD) ? dti->containee.size_def.bit_field_size : 0)
uint32_t get_prefix_bitmap_size_in_bits_for_container_type_info(const data_type_info* dti, const void* data);
#define get_prefix_bitmap_size_for_container_type_info(dti, data) 					(bitmap_size_in_bytes(get_prefix_bitmap_size_in_bits_for_container_type_info(dti, data)))

// valid for string, blob, tuple and array (generated on the fly for an array)
// valid only if index < get_element_count_for_container_type_info
data_position_info get_data_position_info_for_container(const data_type_info* dti, const void* data, uint32_t index);
