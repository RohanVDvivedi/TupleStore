#include<tuple.h>

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

uint32_t get_element_size(const tuple_def* tpl_d, uint16_t index, const void* tupl)
{
	if(tpl_d->element_defs[index].size != VARIABLE_SIZED)
		return tpl_d->element_defs[index].size;
	else
	{
		switch(tpl_d->element_defs[index - 1].size)
		{
			case 1 :
				return (*(seek_to_element(tpl_d, index - 1, tupl).UINT_1));
			case 2 :
				return (*(seek_to_element(tpl_d, index - 1, tupl).UINT_2));
			case 4 :
				return (*(seek_to_element(tpl_d, index - 1, tupl).UINT_4));

			// this is the error case it may never occur
			default:
				return 0;
		}
	}
}

uint32_t get_element_offset(const tuple_def* tpl_d, uint16_t index, const void* tupl)
{
	if(tpl_d->size != VARIABLE_SIZED) // i.e. fixed sized
		return tpl_d->element_defs[index].byte_offset;
	else
	{
		uint32_t offset = 0;

		//#define USE_DYNAMIC_PROGRAMMING_APPROACH

		#ifdef USE_DYNAMIC_PROGRAMMING_APPROACH
			// TODO
		#else	// loop over all the elements (until the index) and add their sizes

			for(uint16_t i = 0; i < index; i++)
				offset += get_element_size(tpl_d, i, tupl);

		#endif

		return offset;
	}
}

element seek_to_element(const tuple_def* tpl_d, uint16_t index, const void* tupl)
{
	return (element){.BLOB = (void*)(tupl + get_element_offset(tpl_d, index, tupl))};
}

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl)
{
	if(tpl_d->size != VARIABLE_SIZED) // i.e. fixed sized tuple
		return tpl_d->size;
	else
	{
		// for VARIABLE_SIZED tuple return last_element's offset + last_element's size
		uint16_t last_index = tpl_d->element_count - 1;
		return get_element_offset(tpl_d, last_index, tupl) + get_element_size(tpl_d, last_index, tupl);
	}
}

void* seek_to_end_of_tuple(const tuple_def* tpl_d, const void* tupl)
{
	return (void*)(tupl + get_tuple_size(tpl_d, tupl));
}

void copy_element_to_tuple(const tuple_def* tpl_d, uint16_t index, void* tupl, const void* value)
{
	element ele = seek_to_element(tpl_d, index, tupl);
	memmove(ele.BLOB, value, get_element_size(tpl_d, index, tupl));
}

void copy_element_from_tuple(const tuple_def* tpl_d, uint16_t index, const void* tupl, void* value)
{
	element ele = seek_to_element(tpl_d, index, tupl);
	memmove(value, ele.BLOB, get_element_size(tpl_d, index, tupl));
}

int compare_elements(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint16_t index)
{
	// seek to the elements to be compared
	element e1 = seek_to_element(tpl_d, index, tup1);
	element e2 = seek_to_element(tpl_d, index, tup2);

	// if fixed sized elements compare them directly
	if(tpl_d->element_defs[index].size != VARIABLE_SIZED)
		return compare_fixed_sized_elements(e1, e2, tpl_d->element_defs + index);
	else
	{
		element_type element_compare_type = tpl_d->element_defs[index].type;

		uint32_t size1 = get_element_size(tpl_d, index, tup1);
		uint32_t size2 = get_element_size(tpl_d, index, tup2);

		uint32_t min_size = (size1 < size2) ? size1 : size2;

		int compare = 0;
		if(element_compare_type == STRING)
			compare = strncmp(e1.STRING, e2.STRING, min_size);
		else if(element_compare_type == BLOB)
			compare = memcmp(e1.STRING, e2.STRING, min_size);

		if(compare > 0)
			compare = 1;
		else if(compare < 0)
			compare = -1;
		else if((compare == 0) && (size1 != size2))
		{
			// in dictionary ordering if 1 string is a prefix of the other
			// then the larger string comes latter in the order
			if(size1 > size2)
				compare = -1;
			else if(size1 > size2)
				compare = 1;
		}

		return compare;
	}
}

int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d)
{
	int compare = 0;
	for(uint16_t i = 0; ((i < tpl_d->element_count) && (compare == 0)); i++)
	{
		// SKIP THE ELEMENT IF IT COMES BEFORE A VARIABLE SIZED ELEMENT, SINCE THIS ELEMENT IS NOT ACTUAL DATA
		// IT IS ONLY NEEDED TO READ THE SIZE OF THE VARIABLE SIZED DATA

		// do not compare a size specifying element
		if(is_size_specifying_element(tpl_d, i))
			continue;

		compare = compare_elements(tup1, tup2, tpl_d, i);
	}
	return compare;
}

uint32_t hash_element(const void* tup, const tuple_def* tpl_d, uint16_t index, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	// seek to the elements to be compared
	element ele = seek_to_element(tpl_d, index, tup);

	// size of the element
	uint32_t size = get_element_size(tpl_d, index, tup);

	// for a STRING type the size is the capacity, not the actual size, 
	// the string may be smaller than the size
	if(tpl_d->element_defs[index].type == STRING)
		return hash_func(ele.STRING, strnlen(ele.STRING, size));
	else
		return hash_func(ele.BLOB, size);
}

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	uint32_t hash_value = 0;
	for(uint16_t i = 0; i < tpl_d->element_count; i++)
	{
		// SKIP THE ELEMENT IF IT COMES BEFORE A VARIABLE SIZED ELEMENT, SINCE THIS ELEMENT IS NOT ACTUAL DATA
		// IT IS ONLY NEEDED TO READ THE SIZE OF THE VARIABLE SIZED DATA

		// do not hash a size specifying element
		if(is_size_specifying_element(tpl_d, i))
			continue;

		hash_value += hash_element(tup, tpl_d, i, hash_func);
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
	for(uint16_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
			chars_written += sprintf(str + chars_written, ", ");


		element e = seek_to_element(tpl_d, i, tup);
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
				uint32_t size = get_element_size(tpl_d, i, tup);
				chars_written += sprintf(str + chars_written, "\"%.*s\"", size, e.STRING);
				break;
			}
			case BLOB :
			{
				uint32_t size = get_element_size(tpl_d, i, tup);
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
	for(uint16_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
		{
			sscanf(str + chars_read, ", %n", &nr);						chars_read += nr;
		}

		element e = seek_to_element(tpl_d, i, tup);
		switch(tpl_d->element_defs[i].type)
		{
			case UINT :
			{
				u8 temp;
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
				i8 temp;
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
						f4 temp;
						sscanf(str + chars_read, "%f%n", &temp, &nr); 	chars_read += nr;
						(*(e.FLOAT_4)) = temp;
						break;
					}
					case 8 :
					{
						f8 temp;
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