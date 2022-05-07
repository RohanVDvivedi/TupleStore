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

#define min(a,b) (((a)<(b))?(a):(b))

void set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t value_size)
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
						write_value_to(tupl + tpl_d->element_defs[i].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, offset - old_total_size);
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

			uint32_t tuple_size = get_tuple_size(tpl_d, tupl);

			// write offset to 0, for element at index
			write_value_to(tupl + tpl_d->element_defs[index].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets, tuple_size);

			// since the offset is set appropriately and the is_null bit is reset to 0
			// we can access element directly
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
			write_value_to(tupl, tpl_d->size_of_byte_offsets, tuple_size);
		}
	}
}

static void typecast_and_set_numeric_type(element e_to, const element_def* ele_d_to, element e_from, const element_def* ele_d_from);

int set_element_in_tuple_from_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const tuple_def* tpl_d_in, uint32_t index_in, void* tupl_in)
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
	element e1 = get_element_from_tuple(tpl_d1, index1, tup1);
	element e2 = get_element_from_tuple(tpl_d2, index2, tup2);

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
	element e = get_element_from_tuple(tpl_d, index, tup);

	// hashing a NULL element returns a 0
	if(e.BLOB == NULL)
		return 0;
	return hash_element(e, tpl_d->element_defs + index, hash_func);
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

		if(e.BLOB == NULL)
		{
			chars_written += sprintf(str + chars_written, "NULL");
			continue;
		}

		if(is_variable_sized_element_def(tpl_d->element_defs + i))
			chars_written += sprintf(str + chars_written, "[%u]->", read_value_from(tup + tpl_d->element_defs[i].byte_offset_to_byte_offset, tpl_d->size_of_byte_offsets));

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

static void typecast_and_set_numeric_type(element e_to, const element_def* ele_d_to, element e_from, const element_def* ele_d_from)
{
	switch(ele_d_to->type)
	{
		case UINT :
		{
			switch(ele_d_to->size)
			{
				case 1 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_1 = *e_from.UINT_1; return; }
								case 2 : { *e_to.UINT_1 = *e_from.UINT_2; return; }
								case 4 : { *e_to.UINT_1 = *e_from.UINT_4; return; }
								case 8 : { *e_to.UINT_1 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_1 = *e_from.INT_1; return; }
								case 2 : { *e_to.UINT_1 = *e_from.INT_2; return; }
								case 4 : { *e_to.UINT_1 = *e_from.INT_4; return; }
								case 8 : { *e_to.UINT_1 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.UINT_1 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.UINT_1 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_2 = *e_from.UINT_1; return; }
								case 2 : { *e_to.UINT_2 = *e_from.UINT_2; return; }
								case 4 : { *e_to.UINT_2 = *e_from.UINT_4; return; }
								case 8 : { *e_to.UINT_2 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_2 = *e_from.INT_1; return; }
								case 2 : { *e_to.UINT_2 = *e_from.INT_2; return; }
								case 4 : { *e_to.UINT_2 = *e_from.INT_4; return; }
								case 8 : { *e_to.UINT_2 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.UINT_2 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.UINT_2 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_4 = *e_from.UINT_1; return; }
								case 2 : { *e_to.UINT_4 = *e_from.UINT_2; return; }
								case 4 : { *e_to.UINT_4 = *e_from.UINT_4; return; }
								case 8 : { *e_to.UINT_4 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_4 = *e_from.INT_1; return; }
								case 2 : { *e_to.UINT_4 = *e_from.INT_2; return; }
								case 4 : { *e_to.UINT_4 = *e_from.INT_4; return; }
								case 8 : { *e_to.UINT_4 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.UINT_4 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.UINT_4 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_8 = *e_from.UINT_1; return; }
								case 2 : { *e_to.UINT_8 = *e_from.UINT_2; return; }
								case 4 : { *e_to.UINT_8 = *e_from.UINT_4; return; }
								case 8 : { *e_to.UINT_8 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.UINT_8 = *e_from.INT_1; return; }
								case 2 : { *e_to.UINT_8 = *e_from.INT_2; return; }
								case 4 : { *e_to.UINT_8 = *e_from.INT_4; return; }
								case 8 : { *e_to.UINT_8 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.UINT_8 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.UINT_8 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
			}
		}
		case INT :
		{
			switch(ele_d_to->size)
			{
				case 1 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_1 = *e_from.UINT_1; return; }
								case 2 : { *e_to.INT_1 = *e_from.UINT_2; return; }
								case 4 : { *e_to.INT_1 = *e_from.UINT_4; return; }
								case 8 : { *e_to.INT_1 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_1 = *e_from.INT_1; return; }
								case 2 : { *e_to.INT_1 = *e_from.INT_2; return; }
								case 4 : { *e_to.INT_1 = *e_from.INT_4; return; }
								case 8 : { *e_to.INT_1 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.INT_1 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.INT_1 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_2 = *e_from.UINT_1; return; }
								case 2 : { *e_to.INT_2 = *e_from.UINT_2; return; }
								case 4 : { *e_to.INT_2 = *e_from.UINT_4; return; }
								case 8 : { *e_to.INT_2 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_2 = *e_from.INT_1; return; }
								case 2 : { *e_to.INT_2 = *e_from.INT_2; return; }
								case 4 : { *e_to.INT_2 = *e_from.INT_4; return; }
								case 8 : { *e_to.INT_2 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.INT_2 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.INT_2 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_4 = *e_from.UINT_1; return; }
								case 2 : { *e_to.INT_4 = *e_from.UINT_2; return; }
								case 4 : { *e_to.INT_4 = *e_from.UINT_4; return; }
								case 8 : { *e_to.INT_4 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_4 = *e_from.INT_1; return; }
								case 2 : { *e_to.INT_4 = *e_from.INT_2; return; }
								case 4 : { *e_to.INT_4 = *e_from.INT_4; return; }
								case 8 : { *e_to.INT_4 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.INT_4 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.INT_4 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_8 = *e_from.UINT_1; return; }
								case 2 : { *e_to.INT_8 = *e_from.UINT_2; return; }
								case 4 : { *e_to.INT_8 = *e_from.UINT_4; return; }
								case 8 : { *e_to.INT_8 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.INT_8 = *e_from.INT_1; return; }
								case 2 : { *e_to.INT_8 = *e_from.INT_2; return; }
								case 4 : { *e_to.INT_8 = *e_from.INT_4; return; }
								case 8 : { *e_to.INT_8 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.INT_8 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.INT_8 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
			}
		}
		case FLOAT :
		{
			switch(ele_d_to->size)
			{
				case 4 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.FLOAT_4 = *e_from.UINT_1; return; }
								case 2 : { *e_to.FLOAT_4 = *e_from.UINT_2; return; }
								case 4 : { *e_to.FLOAT_4 = *e_from.UINT_4; return; }
								case 8 : { *e_to.FLOAT_4 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.FLOAT_4 = *e_from.INT_1; return; }
								case 2 : { *e_to.FLOAT_4 = *e_from.INT_2; return; }
								case 4 : { *e_to.FLOAT_4 = *e_from.INT_4; return; }
								case 8 : { *e_to.FLOAT_4 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.FLOAT_4 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.FLOAT_4 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_from->type)
					{
						case UINT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.FLOAT_8 = *e_from.UINT_1; return; }
								case 2 : { *e_to.FLOAT_8 = *e_from.UINT_2; return; }
								case 4 : { *e_to.FLOAT_8 = *e_from.UINT_4; return; }
								case 8 : { *e_to.FLOAT_8 = *e_from.UINT_8; return; }
							}
						}
						case INT :
						{
							switch(ele_d_from->size)
							{
								case 1 : { *e_to.FLOAT_8 = *e_from.INT_1; return; }
								case 2 : { *e_to.FLOAT_8 = *e_from.INT_2; return; }
								case 4 : { *e_to.FLOAT_8 = *e_from.INT_4; return; }
								case 8 : { *e_to.FLOAT_8 = *e_from.INT_8; return; }
							}
						}
						case FLOAT :
						{
							switch(ele_d_from->size)
							{
								case 4 : { *e_to.FLOAT_8 = *e_from.FLOAT_4; return; }
								case 8 : { *e_to.FLOAT_8 = *e_from.FLOAT_8; return; }
							}
						}
						default : {break;}
					}
				}
			}
		}
		default : {break;}
	}
}