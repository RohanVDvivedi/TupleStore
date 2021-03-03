#include<tuple.h>

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

uint64_t get_element_size(tuple_def* tpl_d, uint64_t index, const void* tupl)
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
			case 8 :
				return (*(seek_to_element(tpl_d, index - 1, tupl).UINT_8));

			// this is the error case it may never occur
			default:
				return 0;
		}
	}
}

uint64_t get_element_offset(tuple_def* tpl_d, uint64_t index, const void* tupl)
{
	if(tpl_d->size != VARIABLE_SIZED) // i.e. fixed sized
		return tpl_d->element_defs[index].byte_offset;
	else
	{
		uint64_t offset = 0;

		//#define USE_DYNAMIC_PROGRAMMING_APPROACH

		#ifdef USE_DYNAMIC_PROGRAMMING_APPROACH
			
		#else	// loop over all the elements (until the index) and add their sizes

			for(uint16_t i = 0; i < index; i++)
				offset += get_element_size(tpl_d, i, tupl);

		#endif

		return offset;
	}
}

element seek_to_element(tuple_def* tpl_d, uint64_t index, const void* tupl)
{
	return (element){.BLOB = (void*)(tupl + get_element_offset(tpl_d, index, tupl))};
}

void copy_element_to_tuple(tuple_def* tpl_d, uint64_t index, void* tupl, const void* value)
{
	element ele = seek_to_element(tpl_d, index, tupl);
	memmove(ele.BLOB, value, get_element_size(tpl_d, index, tupl));
}

void copy_element_from_tuple(tuple_def* tpl_d, uint64_t index, const void* tupl, void* value)
{
	element ele = seek_to_element(tpl_d, index, tupl);
	memmove(value, ele.BLOB, get_element_size(tpl_d, index, tupl));
}

#define compare(a,b)	( ((a)>(b)) ? 1 : (((a)<(b)) ? (-1) : 0 ) )

int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d)
{
	return 0;
}

int sprint_tuple(char* str, void* tup, tuple_def* tpl_d)
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
			case BLOB :
			case STRING :
			{
				chars_written += sprintf(str + chars_written, "%s", e.STRING);
				break;
			}
		}
	}
	chars_written += sprintf(str + chars_written, "\n");
	return chars_written;
}

int sscan_tuple(char* str, void* tup, tuple_def* tpl_d)
{
	int nr = 0;
	int chars_read = 0;
	for(uint16_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
			sscanf(str + chars_read, ", %n", &nr);						chars_read += nr;


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