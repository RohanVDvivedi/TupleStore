#include<tuple.h>

#include<stdio.h>
#include<alloca.h>
#include<string.h>

#include<bitmap.h>

#include<tuple_def.h>
#include<page_layout_util.h>

void init_tuple(const tuple_def* tpl_d, void* tupl)
{
	set_all_bits(tupl + tpl_d->byte_offset_to_is_null_bitmap, tpl_d->element_count);

	// set its size to min_size
	if(is_variable_sized_tuple_def(tpl_d))
		write_value_to(tupl, tpl_d->size_of_byte_offsets, tpl_d->min_size);
}

uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	return get_element_size(get_element_from_tuple(tpl_d, index, tupl), tpl_d->element_defs + index);
}

uint32_t get_element_offset_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	if(is_fixed_sized_element_def(tpl_d->element_defs + index)) // i.e. fixed sized
		return tpl_d->element_defs[index].byte_offset;
	else
		return read_value_from(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets);
}

element get_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	// return a NULL if the element id NULL
	if(is_NULL_in_tuple(tpl_d, index, tupl))
		return (element){.BLOB = NULL};

	return (element){.BLOB = (void*)(tupl + get_element_offset_within_tuple(tpl_d, index, tupl))};
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	if(is_fixed_sized_tuple_def(tpl_d)) // i.e. fixed sized tuple
		return tpl_d->size;
	else
		return read_value_from(tupl, tpl_d->size_of_byte_offsets);
}

void* get_end_of_tuple(const tuple_def* tpl_d, const void* tupl)
{
	return (void*)(tupl + get_tuple_size(tpl_d, tupl));
}

int is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	return get_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
}

void set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t var_blob_size)
{
	element existing = get_element_from_tuple(tpl_d, index, tupl);
	if(existing.BLOB == NULL && value == NULL)
		return;

	if(is_fixed_sized_element_def(tpl_d->element_defs + index))
	{
		// if the value to be set is NULL, then just set the corresponding bit in the is_null bitmap
		if(value == NULL)
			set_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		else
		{
			// set the is_null bitmap bit to 0
			reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);

			// calculate total size occupied by the fixed length data type
			uint32_t total_size = get_element_size_within_tuple(tpl_d, index, tupl);
			if(tpl_d->element_defs[index].type == STRING)
			{
				// copy at most of string length + 1 bytes and total_size
				uint32_t string_size = strnlen(value, total_size) + 1;
				uint32_t copy_size = (total_size < string_size) ? total_size : string_size;
				memmove(existing.STRING, value, copy_size);
			}
			else
				memmove(existing.BLOB, value, total_size);
		}
	}
	else
	{
		// if data existed at index
		if(existing.BLOB != NULL)
		{
			uint32_t old_total_size = get_element_size_within_tuple(tpl_d, index, tupl);
			uint32_t old_offset = get_element_offset_within_tuple(tpl_d, index, tupl);

			uint32_t old_tuple_size = get_tuple_size(tpl_d, tupl);

			// move all the bytes in the tuple after this element to front
			memmove(tupl + old_offset, tupl + old_offset + old_total_size, old_tuple_size - (old_offset + old_total_size));

			// decrease all tuple offsets for variable sized elements that were after the element by old_total_size
			for(uint32_t i = 0; i < tpl_d->element_count; i++)
			{
				if(is_variable_sized_element_def(tpl_d->element_defs + i))
				{
					uint32_t offset = read_value_from(tupl + tpl_d->element_defs[i].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets);
					if(offset > old_offset)
						write_value_to(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, offset - old_total_size);
				}
			}

			// update tuple size to old_tuple_size - old_total_size
			write_value_to(tupl, tpl_d->size_of_byte_offsets, old_tuple_size - old_total_size);

			// write offset to 0, for element at index
			write_value_to(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, 0);
			
			// set is_null bit, for element at index
			set_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		}

		// set the new data with value if it is not NULL 
		if(value != NULL)
		{
			// set the is_null bitmap bit to 0
			reset_bit(tupl + tpl_d->byte_offset_to_is_null_bitmap, index);
		}
	}
}

static void copy_element_to_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t var_blob_size)
{
	element ele = get_element_from_tuple(tpl_d, index, tupl);

	if(is_fixed_sized_element_def(tpl_d->element_defs + index))
	{
		uint32_t total_size = get_element_size_within_tuple(tpl_d, index, tupl);
		if(tpl_d->element_defs[index].type == STRING)
		{
			uint32_t string_size = strnlen(value, total_size) + 1;
			uint32_t copy_size = (total_size < string_size) ? total_size : string_size;
			memmove(ele.STRING, value, copy_size);
		}
		else
			memmove(ele.BLOB, value, total_size);
	}
	else
	{
		switch(tpl_d->element_defs[index].type)
		{
			case VAR_STRING :
			{
				switch(tpl_d->element_defs[index].size_specifier_prefix_size)
				{
					case 1 :
					{
						ele.VAR_STRING_1->size = strnlen(value, (1<<8)-1);;
						memmove(ele.VAR_STRING_1->string, value, ele.VAR_STRING_1->size);
						break;
					}
					case 2 :
					{
						ele.VAR_STRING_2->size = strnlen(value, (1<<16)-1);;
						memmove(ele.VAR_STRING_2->string, value, ele.VAR_STRING_2->size);
						break;
					}
					case 4 :
					{
						ele.VAR_STRING_4->size = strlen(value);
						memmove(ele.VAR_STRING_4->string, value, ele.VAR_STRING_4->size);
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
						ele.VAR_BLOB_1->size = (var_blob_size > ((1<<8)-1)) ? ((1<<8)-1) : var_blob_size;
						memmove(ele.VAR_BLOB_1->blob, value, ele.VAR_BLOB_1->size);
						break;
					}
					case 2 :
					{
						ele.VAR_BLOB_2->size = (var_blob_size > ((1<<16)-1)) ? ((1<<16)-1) : var_blob_size;
						memmove(ele.VAR_BLOB_2->blob, value, ele.VAR_BLOB_2->size);
						break;
					}
					case 4 :
					{
						ele.VAR_BLOB_4->size = var_blob_size;
						memmove(ele.VAR_BLOB_4->blob, value, ele.VAR_BLOB_4->size);
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

void copy_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl, void* value)
{
	element ele = get_element_from_tuple(tpl_d, index, tupl);
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

int compare_elements_within_tuple(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t index)
{
	element e1 = get_element_from_tuple(tpl_d, index, tup1);
	element e2 = get_element_from_tuple(tpl_d, index, tup2);

	// handling case when elements are NULL
	if(e1.BLOB == NULL && e2.BLOB == NULL)
		return 0;
	else if(e1.BLOB == NULL || e2.BLOB == NULL)
	{
		// a NULL element is always considered lesser than a NON NULL element
		if(e1.BLOB == NULL)
			return -1;
		else
			return 1;
	}
	else
		return compare_elements(e1, e2, tpl_d->element_defs + index);
}

int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t element_count, uint32_t* element_ids)
{
	int compare = 0;
	if(element_ids == NULL)
	{
		for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
			compare = compare_elements_within_tuple(tup1, tup2, tpl_d, i);
	}
	else
	{
		for(uint32_t i = 0; ((i < element_count) && (compare == 0)); i++)
			compare = compare_elements_within_tuple(tup1, tup2, tpl_d, element_ids[i]);
	}
	return compare;
}

uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	element e = get_element_from_tuple(tpl_d, index, tup);

	// hashing a NULL element returns a 0
	if(e.BLOB == NULL)
		return 0;
	return hash_element(e, tpl_d->element_defs + index, hash_func);
}

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count, uint32_t* element_ids)
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

int sprint_tuple(char* str, const void* tup, const tuple_def* tpl_d)
{
	if(tup == NULL)
	{
		char* empty = "Empty\n";
		strcpy(str, empty);
		return 4; 
	}
	int chars_written = 0;
	for(uint32_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
			chars_written += sprintf(str + chars_written, ", ");


		element e = get_element_from_tuple(tpl_d, i, tup);
		switch(tpl_d->element_defs[i].type)
		{
			case UINT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						chars_written += sprintf(str + chars_written, "%u",  *(e.UINT_1));
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%u",  *(e.UINT_2));
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%u", *(e.UINT_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%lu", *(e.UINT_8));
						break;
					}
				}
				break;
			}
			case INT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.INT_1));
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.INT_2));
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.INT_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%ld", *(e.INT_8));
						break;
					}
				}
				break;
			}
			case FLOAT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%f", *(e.FLOAT_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%lf", *(e.FLOAT_8));
						break;
					}
				}
				break;
			}
			case STRING :
			{
				uint32_t size = get_element_size_within_tuple(tpl_d, i, tup);
				chars_written += sprintf(str + chars_written, "\"%.*s\"", size, e.STRING);
				break;
			}
			case BLOB :
			{
				uint32_t size = get_element_size_within_tuple(tpl_d, i, tup);
				chars_written += sprintf(str + chars_written, "BLOB(%u)[", size);
				for(uint32_t i = 0; i < size; i++)
					chars_written += sprintf(str + chars_written, " 0x%2x", (*((uint8_t*)(e.BLOB + i))));
				chars_written += sprintf(str + chars_written, "]");
				break;
			}
			case VAR_STRING :
			{
				switch(tpl_d->element_defs[i].size_specifier_prefix_size)
				{
					case 1 :
					{
						chars_written += sprintf(str + chars_written, "\"%.*s\"", e.VAR_STRING_1->size, e.VAR_STRING_1->string);
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "\"%.*s\"", e.VAR_STRING_2->size, e.VAR_STRING_2->string);
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "\"%.*s\"", e.VAR_STRING_4->size, e.VAR_STRING_4->string);
						break;
					}
				}
				break;
			}
			case VAR_BLOB :
			{
				uint32_t size = 0;
				char* blob_data = NULL;
				switch(tpl_d->element_defs[i].size_specifier_prefix_size)
				{
					case 1 :
					{
						size = e.VAR_BLOB_1->size;
						blob_data = e.VAR_BLOB_1->blob;
						break;
					}
					case 2 :
					{
						size = e.VAR_BLOB_2->size;
						blob_data = e.VAR_BLOB_2->blob;
						break;
					}
					case 4 :
					{
						size = e.VAR_BLOB_4->size;
						blob_data = e.VAR_BLOB_4->blob;
						break;
					}
				}
				chars_written += sprintf(str + chars_written, "BLOB(%u)[", size);
				for(uint32_t i = 0; i < size; i++)
					chars_written += sprintf(str + chars_written, " 0x%2x", *((uint8_t*)(blob_data + i)));
				chars_written += sprintf(str + chars_written, "]");
				break;
			}
		}
	}
	chars_written += sprintf(str + chars_written, "\n");
	return chars_written;
}