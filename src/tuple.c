#include<tuple.h>

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	return get_element_size(get_element_from_tuple(tpl_d, index, tupl), tpl_d->element_defs + index);
}

uint32_t get_element_offset_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	if(is_fixed_sized_tuple_def(tpl_d)) // i.e. fixed sized
		return tpl_d->element_defs[index].byte_offset;
	else
	{
		uint32_t offset = 0;

		//#define USE_DYNAMIC_PROGRAMMING_APPROACH

		#ifdef USE_DYNAMIC_PROGRAMMING_APPROACH
			// TODO
		#else	// loop over all the elements (until the index) and add their sizes

			for(uint32_t i = 0; i < index; i++)
				offset += get_element_size_within_tuple(tpl_d, i, tupl);

		#endif

		return offset;
	}
}

element get_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl)
{
	return (element){.BLOB = (void*)(tupl + get_element_offset_within_tuple(tpl_d, index, tupl))};
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	if(is_fixed_sized_tuple_def(tpl_d)) // i.e. fixed sized tuple
		return tpl_d->size;
	else
	{
		// for VARIABLE_SIZED tuple return last_element's offset + last_element's size
		uint32_t last_index = tpl_d->element_count - 1;
		return get_element_offset_within_tuple(tpl_d, last_index, tupl) + get_element_size_within_tuple(tpl_d, last_index, tupl);
	}
}

void* seek_to_end_of_tuple(const tuple_def* tpl_d, const void* tupl)
{
	return (void*)(tupl + get_tuple_size(tpl_d, tupl));
}

void copy_element_to_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t var_blob_size)
{
	element ele = get_element_from_tuple(tpl_d, index, tupl);

	if(is_fixed_sized_element_def(tpl_d->element_defs + index))
	{
		if(tpl_d->element_defs[index].type == STRING)
		{
			uint32_t total_size = get_element_size_within_tuple(tpl_d, index, tupl);
			uint32_t string_size = strnlen(value, total_size) + 1;
			uint32_t copy_size = (total_size < string_size) ? total_size : string_size;
			memmove(ele.STRING, value, copy_size);
		}
		else
			memmove(ele.BLOB, value, get_element_size_within_tuple(tpl_d, index, tupl));
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
						uint32_t copy_size = strnlen(value, (1<<8)-1);
						ele.VAR_STRING_1->size = copy_size;
						memmove(ele.VAR_STRING_1->string, value, copy_size);
						break;
					}
					case 2 :
					{
						uint32_t copy_size = strnlen(value, (1<<16)-1);
						ele.VAR_STRING_2->size = copy_size;
						memmove(ele.VAR_STRING_2->string, value, copy_size);
						break;
					}
					case 4 :
					{
						uint32_t copy_size = strnlen(value, (1<<32)-1);
						ele.VAR_STRING_4->size = copy_size;
						memmove(ele.VAR_STRING_4->string, value, copy_size);
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
						var_blob_size = (var_blob_size > ((1<<8)-1)) ? ((1<<8)-1) : var_blob_size;
						ele.VAR_STRING_1->size = var_blob_size;
						memmove(ele.VAR_BLOB_1->blob, value, var_blob_size);
						break;
					}
					case 2 :
					{
						var_blob_size = (var_blob_size > ((1<<16)-1)) ? ((1<<16)-1) : var_blob_size;
						ele.VAR_STRING_2->size = var_blob_size;
						memmove(ele.VAR_BLOB_2->blob, value, var_blob_size);
						break;
					}
					case 4 :
					{
						ele.VAR_STRING_4->size = var_blob_size;
						memmove(ele.VAR_BLOB_4->blob, value, var_blob_size);
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
	memmove(value, ele.BLOB, get_element_size_within_tuple(tpl_d, index, tupl));
}

int compare_elements_within_tuple(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t index)
{
	// seek to the elements to be compared
	element e1 = get_element_from_tuple(tpl_d, index, tup1);
	element e2 = get_element_from_tuple(tpl_d, index, tup2);
	return compare_elements(e1, e2, tpl_d->element_defs + index);
}

int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d)
{
	int compare = 0;
	for(uint32_t i = 0; ((i < tpl_d->element_count) && (compare == 0)); i++)
	{
		compare = compare_elements_within_tuple(tup1, tup2, tpl_d, i);
	}
	return compare;
}

uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	element e = get_element_from_tuple(tpl_d, index, tup);
	return hash_element(e, tpl_d->element_defs + index, hash_func);
}

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	uint32_t hash_value = 0;
	for(uint32_t i = 0; i < tpl_d->element_count; i++)
	{
		hash_value += hash_element_within_tuple(tup, tpl_d, i, hash_func);
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
		}
	}
	chars_written += sprintf(str + chars_written, "\n");
	return chars_written;
}

int sscan_tuple(const char* str, void* tup, const tuple_def* tpl_d)
{
	int nr = 0;
	int chars_read = 0;
	for(uint32_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
		{
			sscanf(str + chars_read, ", %n", &nr);						chars_read += nr;
		}

		element e = get_element_from_tuple(tpl_d, i, tup);
		switch(tpl_d->element_defs[i].type)
		{
			case UINT :
			{
				uint64_t temp;
				sscanf(str + chars_read, "%lu%n", &temp, &nr);			chars_read += nr;
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						(*(e.UINT_1)) = temp & 0xff;
						break;
					}
					case 2 :
					{
						(*(e.UINT_2)) = temp & 0xffff;
						break;
					}
					case 4 :
					{
						(*(e.UINT_4)) = temp & 0xffffffff;
						break;
					}
					case 8 :
					{
						(*(e.UINT_8)) = temp;
						break;
					}
				}
				break;
			}
			case INT :
			{
				int64_t temp;
				sscanf(str + chars_read, "%ld%n", &temp, &nr);			chars_read += nr;
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						(*(e.INT_1)) = temp & 0xff;
						break;
					}
					case 2 :
					{
						(*(e.INT_2)) = temp & 0xffff;
						break;
					}
					case 4 :
					{
						(*(e.INT_4)) = temp & 0xffffffff;
						break;
					}
					case 8 :
					{
						(*(e.INT_8)) = temp;
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
						float temp;
						sscanf(str + chars_read, "%f%n", &temp, &nr); 	chars_read += nr;
						(*(e.FLOAT_4)) = temp;
						break;
					}
					case 8 :
					{
						double temp;
						sscanf(str + chars_read, "%lf%n", &temp, &nr);	chars_read += nr;
						(*(e.FLOAT_8)) = temp;
						break;
					}
				}
				break;
			}
			case BLOB :
			case STRING :
			{
				sscanf(str + chars_read, "%[^,]%n", e.STRING, &nr);	chars_read += nr;
				break;
			}
		}
	}
	sscanf(str + chars_read, "\n%n", &nr); 	chars_read += nr;
	return chars_read;
}