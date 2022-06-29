#include<tuple.h>

#include<stdio.h>
#include<inttypes.h>
#include<alloca.h>
#include<string.h>

#include<bitmap.h>

#include<tuple_def.h>
#include<int_accesses.h>
#include<numeral_element_types.h>
#include<non_numeral_element_types.h>
#include<page_layout_util.h>

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	set_all_bits(tupl + tpl_d->byte_offset_to_is_null_bitmap, tpl_d->element_count);

	// set its size to min_size
	if(is_variable_sized_tuple_def(tpl_d))
		write_uint32(tupl, tpl_d->size_of_byte_offsets, tpl_d->min_size);
}

// do not use this function directly, call get_element_from_tuple macro
static uint32_t get_element_offset_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	const element_def* ele_d = tpl_d->element_defs + index;
	if(is_fixed_sized_element_def(ele_d)) // i.e. fixed sized
		return ele_d->byte_offset;
	else
		return read_uint32(tupl + ele_d->byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets);
}

#define get_element_from_tuple(tpl_d, index, tupl) (is_NULL_in_tuple(tpl_d, index, tupl) ? NULL : ((tupl) + get_element_offset_within_tuple(tpl_d, index, tupl)))

uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	const element_def* ele_d = tpl_d->element_defs + index;

	// for a NULL "variable sized element", no space is allocated for its actual contents (there is only space for its offset (from the first byte of the tuple))
	if(is_variable_sized_element_def(ele_d) && is_NULL_in_tuple(tpl_d, index, tupl))
		return 0;
	else
	{
		const void* e = get_element_from_tuple(tpl_d, index, tupl);
		return get_element_size(e, ele_d);
	}
}

uint32_t get_element_data_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	// if the element is NULL no bytes must be required in user_value to store it
	if(is_NULL_in_tuple(tpl_d, index, tupl))
		return 0;

	const element_def* ele_d = tpl_d->element_defs + index;

	if(is_fixed_sized_element_def(ele_d) && !is_string_type_element_def(ele_d)) // fixed sized except for a string
		return ele_d->size;
	else if(is_string_type_element_def(ele_d)) // fixed sized and a string
	{
		const void* e = get_element_from_tuple(tpl_d, index, tupl);
		return get_string_length_for_string_type_element(e, ele_d) + 1;
	}
	else // it is VAR_BLOB
	{
		const void* e = get_element_from_tuple(tpl_d, index, tupl);
		return get_data_size_for_variable_sized_non_numeral_element(e, ele_d);
	}
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	if(is_fixed_sized_tuple_def(tpl_d)) // i.e. fixed sized tuple
		return tpl_d->size;
	else // for a variable sized tuple the first few bytes are used to store its size
		return read_uint32(tupl, tpl_d->size_of_byte_offsets);
}

int is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	return get_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
}

// this function only sets the is_NULL bit, it is a utility function
// call only set_element_* functions, this function will be used by the set_element_* function internally
static int set_is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, int is_NULL_bit_value)
{
	void* is_NULL_bitmap = tupl + tpl_d->byte_offset_to_is_null_bitmap;
	if(is_NULL_bit_value)
		set_bit(is_NULL_bitmap, index);
	else
		reset_bit(is_NULL_bitmap, index);
	return 1;
}

void set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t value_size)
{
	// if the element inside tuple is NULL, and we are asked to set it to NULL, then return
	if(is_NULL_in_tuple(tpl_d, index, tupl) && value == NULL)
		return;

	if(is_fixed_sized_element_def(tpl_d->element_defs + index))
	{
		// if the value to be set is NULL
		if(value == NULL)
		{
			// here we are sure that the element in the tuple is not NULL

			// reset the corresponding bytes (of the fixed length element)
			uint32_t byte_offset_to_element = get_element_offset_within_tuple(tpl_d, index, tupl);
			uint32_t bytes_occupied_by_element = get_element_size_within_tuple(tpl_d, index, tupl);
			memset(tupl + byte_offset_to_element, 0, bytes_occupied_by_element);

			// then just set the corresponding bit in the is_null bitmap
			set_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		}
		else
		{
			// set the is_null bitmap bit to 0
			reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);

			// this won't return a NULL element because, we just resetted this element's is_null_bitmap bit
			element ele = get_element_from_tuple(tpl_d, index, tupl);

			// calculate total size occupied by the fixed length data type
			uint32_t total_size = get_element_size_within_tuple(tpl_d, index, tupl);
			if(tpl_d->element_defs[index].type == STRING)
			{
				// copy at most of string length + 1 bytes and total_size
				uint32_t string_size = strnlen(value, value_size);
				uint32_t copy_size = min(total_size, string_size);
				memmove(ele.STRING, value, copy_size);

				// if the string is going to occupy lesser characters than its size then we better have a null terminating character at the end
				if(copy_size < total_size)
					ele.STRING[copy_size] = '\0';
			}
			else if(tpl_d->element_defs[index].type == BLOB)
			{
				uint32_t copy_size = min(total_size, value_size);
				memmove(ele.BLOB, value, copy_size);
			}
			else
				memmove(ele.BLOB, value, total_size);
		}
	}
	else
	{
		// if data existed at index (is not NULL), then remove it (its allocated space), set its offset to 0 and set it's is_null_bitmap bit to 1
		if(!is_NULL_in_tuple(tpl_d, index, tupl))
		{
			uint32_t old_total_size = get_element_size_within_tuple(tpl_d, index, tupl);	// this will not be 0 because the element is not NULL
			uint32_t old_offset = get_element_offset_within_tuple(tpl_d, index, tupl);

			uint32_t old_tuple_size = get_tuple_size(tpl_d, tupl);

			// move all the bytes in the tuple after this element to front
			memmove(tupl + old_offset, tupl + old_offset + old_total_size, old_tuple_size - (old_offset + old_total_size));

			// decrease all tuple offsets for variable sized elements that were after the element by old_total_size
			for(uint32_t i = 0; i < tpl_d->element_count; i++)
			{
				if(is_variable_sized_element_def(tpl_d->element_defs + i))
				{
					uint32_t offset = read_uint32(tupl + tpl_d->element_defs[i].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets);
					if(offset > old_offset)
						write_uint32(tupl + tpl_d->element_defs[i].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, offset - old_total_size);
				}
			}

			// update tuple size to old_tuple_size - old_total_size
			write_uint32(tupl, tpl_d->size_of_byte_offsets, old_tuple_size - old_total_size);

			// update offset of this (variable sized) element to 0
			write_uint32(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, 0);
			
			// set is_null bit, for element at index
			set_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		}

		// at this point this element is NULL and has no space allocated for it on the tuple

		// now if there is a value to set
		// then reset its is_null_bitmap bit, allocate space for this new (variable sized) element, and set the new data with value
		if(value != NULL)
		{
			// set the is_null bitmap bit of this element to 0
			reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);

			// get current tuple_size
			uint32_t tuple_size = get_tuple_size(tpl_d, tupl);

			// allocate space at the end of the tuple for this element
			// its new offset will be tuple_size
			// update its offset on the tuple
			write_uint32(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, tuple_size);

			// since the offset is set appropriately and the is_null bit is reset to 0
			// we can access element directly and safely
			element ele = get_element_from_tuple(tpl_d, index, tupl);

			switch(tpl_d->element_defs[index].type)
			{
				case VAR_STRING :
				{
					switch(tpl_d->element_defs[index].size_specifier_prefix_size)
					{
						case 1 :
						{
							uint32_t string_size = strnlen(value, value_size);
							ele.VAR_STRING_1->size = min(string_size, (1U<<8)-1);
							memmove(ele.VAR_STRING_1->string, value, ele.VAR_STRING_1->size);
							tuple_size += (ele.VAR_STRING_1->size + 1);
							break;
						}
						case 2 :
						{
							uint32_t string_size = strnlen(value, value_size);
							ele.VAR_STRING_2->size = min(string_size, (1U<<16)-1);
							memmove(ele.VAR_STRING_2->string, value, ele.VAR_STRING_2->size);
							tuple_size += (ele.VAR_STRING_2->size + 2);
							break;
						}
						case 4 :
						{
							ele.VAR_STRING_4->size = strnlen(value, value_size);
							memmove(ele.VAR_STRING_4->string, value, ele.VAR_STRING_4->size);
							tuple_size += (ele.VAR_STRING_4->size + 4);
							break;
						}
					}
					break;
				}
				case VAR_BLOB :
				{
					switch(tpl_d->element_defs[index].size_specifier_prefix_size)
					{
						case 1 :
						{
							ele.VAR_BLOB_1->size = min(value_size, ((1<<8)-1));
							memmove(ele.VAR_BLOB_1->blob, value, ele.VAR_BLOB_1->size);
							tuple_size += (ele.VAR_BLOB_1->size + 1);
							break;
						}
						case 2 :
						{
							ele.VAR_BLOB_2->size = min(value_size, ((1<<16)-1));
							memmove(ele.VAR_BLOB_2->blob, value, ele.VAR_BLOB_2->size);
							tuple_size += (ele.VAR_BLOB_2->size + 2);
							break;
						}
						case 4 :
						{
							ele.VAR_BLOB_4->size = value_size;
							memmove(ele.VAR_BLOB_4->blob, value, ele.VAR_BLOB_4->size);
							tuple_size += (ele.VAR_BLOB_4->size + 4);
							break;
						}
					}
					break;
				}
				default :
				{
					break;
				}
			}

			// update tuple size to tuple_size
			write_uint32(tupl, tpl_d->size_of_byte_offsets, tuple_size);
		}
	}
}

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const tuple_def* tpl_d_in, uint32_t index_in, const void* tupl_in)
{
	// if the index_in-th element in the tuple is NULL then set index-th element in tuple as NULL
	if(is_NULL_in_tuple(tpl_d_in, index_in, tupl_in))
	{
		set_element_in_tuple(tpl_d, index, tupl, NULL, 0);
		return 1;
	}

	// For numeric types, the type and size of the elements must match up
	if( (tpl_d->element_defs[index].type == UINT || tpl_d->element_defs[index].type == INT || tpl_d->element_defs[index].type == FLOAT)
		&& (tpl_d_in->element_defs[index_in].type == UINT || tpl_d_in->element_defs[index_in].type == INT || tpl_d_in->element_defs[index_in].type == FLOAT) )
	{
		// set the is_null bitmap bit to 0
		reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		
		element e_from = get_element_from_tuple(tpl_d_in, index_in, tupl_in);
		element e_to = get_element_from_tuple(tpl_d, index, tupl);

		typecast_and_set_numeric_type(e_to, tpl_d->element_defs + index, e_from, tpl_d_in->element_defs + index_in);

		return 1;
	}

	// if both of them are string like types then
	else if( (tpl_d->element_defs[index].type == STRING || tpl_d->element_defs[index].type == VAR_STRING)
		&& (tpl_d_in->element_defs[index_in].type == STRING || tpl_d_in->element_defs[index_in].type == VAR_STRING) )
	{
		element existing = get_element_from_tuple(tpl_d_in, index_in, tupl_in);

		switch(tpl_d_in->element_defs[index_in].type)
		{
			case STRING : { set_element_in_tuple(tpl_d, index, tupl, existing.STRING, tpl_d_in->element_defs[index_in].size); break; }
			case VAR_STRING :
			{
				switch(tpl_d_in->element_defs[index_in].size_specifier_prefix_size)
				{
					case 1 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_STRING_1->string, existing.VAR_STRING_1->size); break; }
					case 2 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_STRING_2->string, existing.VAR_STRING_2->size); break; }
					case 4 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_STRING_4->string, existing.VAR_STRING_4->size); break; }
				}
				break;
			}
			default: {break;}
		}

		return 1;
	}

	// if both of them are blob like types then
	else if( (tpl_d->element_defs[index].type == BLOB || tpl_d->element_defs[index].type == VAR_BLOB)
		&& (tpl_d_in->element_defs[index_in].type == BLOB || tpl_d_in->element_defs[index_in].type == VAR_BLOB) )
	{
		element existing = get_element_from_tuple(tpl_d_in, index_in, tupl_in);

		switch(tpl_d_in->element_defs[index_in].type)
		{
			case STRING : { set_element_in_tuple(tpl_d, index, tupl, existing.BLOB, tpl_d_in->element_defs[index_in].size); break; }
			case VAR_STRING :
			{
				switch(tpl_d_in->element_defs[index_in].size_specifier_prefix_size)
				{
					case 1 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_BLOB_1->blob, existing.VAR_BLOB_1->size); break; }
					case 2 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_BLOB_2->blob, existing.VAR_BLOB_2->size); break; }
					case 4 : { set_element_in_tuple(tpl_d, index, tupl, existing.VAR_BLOB_4->blob, existing.VAR_BLOB_4->size); break; }
				}
				break;
			}
			default: {break;}
		}

		return 1;
	}

	return 0;
}

void copy_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl, void* value)
{
	element ele = get_element_from_tuple(tpl_d, index, tupl);

	if(ele.BLOB == NULL)
		return;

	if(is_fixed_sized_element_def(tpl_d->element_defs + index))
	{
		uint32_t total_size = get_element_size_within_tuple(tpl_d, index, tupl);
		if(tpl_d->element_defs[index].type == STRING)
		{
			uint32_t string_size = strnlen(ele.STRING, total_size);
			memmove(value, ele.STRING, string_size);
			((char*)value)[string_size] = '\0';
		}
		else
			memmove(value, ele.BLOB, total_size);
	}
	else
	{
		switch(tpl_d->element_defs[index].type)
		{
			case VAR_STRING :
			{
				uint32_t copy_size = 0;
				switch(tpl_d->element_defs[index].size_specifier_prefix_size)
				{
					case 1 :
					{
						copy_size = strnlen(ele.VAR_STRING_1->string, ele.VAR_STRING_1->size);
						memmove(value, ele.VAR_STRING_1->string, copy_size);
						break;
					}
					case 2 :
					{
						copy_size = strnlen(ele.VAR_STRING_2->string, ele.VAR_STRING_2->size);
						memmove(value, ele.VAR_STRING_2->string, copy_size);
						break;
					}
					case 4 :
					{
						copy_size = strnlen(ele.VAR_STRING_4->string, ele.VAR_STRING_4->size);
						memmove(value, ele.VAR_STRING_4->string, copy_size);
						break;
					}
				}
				((char*)value)[copy_size] = '\0';
				break;
			}
			case VAR_BLOB :
			{
				switch(tpl_d->element_defs[index].size_specifier_prefix_size)
				{
					case 1 :
					{
						memmove(value, ele.VAR_BLOB_1->blob, ele.VAR_BLOB_1->size);
						break;
					}
					case 2 :
					{
						memmove(value, ele.VAR_BLOB_2->blob, ele.VAR_BLOB_2->size);
						break;
					}
					case 4 :
					{
						memmove(value, ele.VAR_BLOB_4->blob, ele.VAR_BLOB_4->size);
						break;
					}
				}
				break;
			}
			default :
			{
				break;
			}
		}

	}
}

int compare_elements_of_tuple(const void* tup1, const tuple_def* tpl_d1, uint32_t index1, const void* tup2, const tuple_def* tpl_d2, uint32_t index2)
{
	const element_def* ele_d1 = tpl_d1->element_defs + index1;
	const element_def* ele_d2 = tpl_d2->element_defs + index2;

	if(!can_compare_element_defs(ele_d1, ele_d2))
		return -2;

	const void* e1 = get_element_from_tuple(tpl_d1, index1, tup1);
	const void* e2 = get_element_from_tuple(tpl_d2, index2, tup2);

	// handling case when elements are NULL
	if(e1 == NULL && e2 == NULL)
		return 0;
	else if(e1 == NULL || e2 == NULL)
	{
		// a NULL element is always considered lesser than a NON NULL element
		if(e1 == NULL)
			return -1;
		else
			return 1;
	}
	else
		return compare_elements(e1, tpl_d1->element_defs + index1, e2, tpl_d2->element_defs + index2);
}

int compare_tuples(const void* tup1, const tuple_def* tpl_d1, const uint32_t* element_ids1, const void* tup2, const tuple_def* tpl_d2, const uint32_t* element_ids2, uint32_t element_count)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
		compare = compare_elements_of_tuple(tup1, tpl_d1, ((element_ids1 == NULL) ? i : element_ids1[i]), tup2, tpl_d2, ((element_ids2 == NULL) ? i : element_ids2[i]));
	return compare;
}

uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	const void* e = get_element_from_tuple(tpl_d, index, tup);

	// hashing a NULL element returns a 0
	if(e == NULL)
		return 0;

	const element_def* ele_d = tpl_d->element_defs + index;
	return hash_element(e, ele_d, hash_func);
}

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count, const uint32_t* element_ids)
{
	uint32_t hash_value = 0;
	if(element_ids == NULL)
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, i, hash_func);
	}
	else
	{
		for(uint32_t i = 0; i < element_count; i++)
			hash_value ^= hash_element_within_tuple(tup, tpl_d, element_ids[i], hash_func);
	}
	return hash_value;
}

uint32_t sprint_tuple(char* str, const void* tup, const tuple_def* tpl_d)
{
	uint32_t chars_written = 0;
	for(uint32_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
			chars_written += sprintf(str + chars_written, ", ");

		const void* e = get_element_from_tuple(tpl_d, i, tup);

		if(e == NULL)
		{
			chars_written += sprintf(str + chars_written, "NULL");
			continue;
		}

		const element_def* ele_d = tpl_d->element_defs + i;

		if(is_variable_sized_element_def(ele_d))
			chars_written += sprintf(str + chars_written, "[%u]->", read_uint32(tup + ele_d->byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets));

		switch(ele_d->type)
		{
			case UINT :
			{
				uint64_t uint_val = read_uint64(e, ele_d->size);
				chars_written += sprintf(str + chars_written, PRIu64, uint_val);
				break;
			}
			case INT :
			{
				int64_t int_val = read_int64(e, ele_d->size);
				chars_written += sprintf(str + chars_written, PRId64, int_val);
				break;
			}
			case FLOAT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 4 :
					{
						float float_val = read_float(e);
						chars_written += sprintf(str + chars_written, "%f", float_val);
						break;
					}
					case 8 :
					{
						float double_val = read_double(e);
						chars_written += sprintf(str + chars_written, "%lf", double_val);
						break;
					}
				}
				break;
			}
			case STRING :
			{
				uint32_t size = get_string_length_for_string_type_element(e, ele_d);
				chars_written += sprintf(str + chars_written, "\"%.*s\"", size, (const char*)e);
				break;
			}
			case BLOB :
			{
				uint32_t size = ele_d->size;
				chars_written += sprintf(str + chars_written, "BLOB(%" PRIu32 ")[", size);
				for(uint32_t i = 0; i < size; i++)
					chars_written += sprintf(str + chars_written, " 0x%2" PRIx8, *(((const uint8_t*)e)+i));
				chars_written += sprintf(str + chars_written, "]");
				break;
			}
			case VAR_STRING :
			{
				const char* data = get_data_for_variable_sized_non_numeral_element(e, ele_d);
				uint32_t size = get_string_length_for_string_type_element(e, ele_d);
				chars_written += sprintf(str + chars_written, "\"%.*s\"", size, data);
				break;
			}
			case VAR_BLOB :
			{
				uint32_t size = get_data_size_for_variable_sized_non_numeral_element(e, ele_d);
				const char* blob_data = get_data_for_variable_sized_non_numeral_element(e, ele_d);
				chars_written += sprintf(str + chars_written, "BLOB(%" PRIu32 ")[", size);
				for(uint32_t i = 0; i < size; i++)
					chars_written += sprintf(str + chars_written, " 0x%2" PRIx8, *((uint8_t*)(blob_data + i)));
				chars_written += sprintf(str + chars_written, "]");
				break;
			}
		}
	}
	chars_written += sprintf(str + chars_written, "\n");
	return chars_written;
}
