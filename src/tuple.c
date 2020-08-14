#include<tuple.h>

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

element seek_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d)
{
	element e;
	e.GENERIC = (tupl + (tpl_d->element_defs[column_no].offset));
	return e;
}

void copy_to_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value)
{
	void* vp = seek_cell(tupl, column_no, tpl_d).GENERIC;
	memcpy(vp, value, tpl_d->element_defs[column_no].size);
}

void copy_from_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value)
{
	void* vp = seek_cell(tupl, column_no, tpl_d).GENERIC;
	memcpy(value, vp, tpl_d->element_defs[column_no].size);
}

int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d)
{
	return 0;
}

void swap_tuples(void* tup1, void* tup2, tuple_def* tpl_d)
{
	void* temp_tupl = alloca(tpl_d->size);
	memcpy(temp_tupl, tup1, tpl_d->size);
	memcpy(tup1, tup2, tpl_d->size);
	memcpy(tup2, temp_tupl, tpl_d->size);
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


		element e = seek_cell(tup, i, tpl_d);
		switch(tpl_d->element_defs[i].type)
		{
			case CHAR_STRING :
			{
				chars_written += sprintf(str + chars_written, "%s", e.CHAR_STRING);
				break;
			}
			case UNSIGNED_INT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						chars_written += sprintf(str + chars_written, "%u",  *(e.UNSIGNED_INT_1));
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%u",  *(e.UNSIGNED_INT_2));
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%u", *(e.UNSIGNED_INT_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%lu", *(e.UNSIGNED_INT_8));
						break;
					}
				}
				break;
			}
			case SIGNED_INT :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.SIGNED_INT_1));
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.SIGNED_INT_2));
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%d", *(e.SIGNED_INT_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%ld", *(e.SIGNED_INT_8));
						break;
					}
				}
				break;
			}
			case FLOATING_NUM :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%f", *(e.FLOATING_NUM_4));
						break;
					}
					case 8 :
					{
						chars_written += sprintf(str + chars_written, "%lf", *(e.FLOATING_NUM_8));
						break;
					}
				}
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


		element e = seek_cell(tup, i, tpl_d);
		switch(tpl_d->element_defs[i].type)
		{
			case CHAR_STRING :
			{
				sscanf(str + chars_read, "%[^,]%n", e.CHAR_STRING, &nr);	chars_read += nr;
				break;
			}
			case UNSIGNED_INT :
			{
				u8 temp;
				sscanf(str + chars_read, "%lu%n", &temp, &nr);			chars_read += nr;
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						(*(e.UNSIGNED_INT_1)) = temp & 0xff;
						break;
					}
					case 2 :
					{
						(*(e.UNSIGNED_INT_2)) = temp & 0xffff;
						break;
					}
					case 4 :
					{
						(*(e.UNSIGNED_INT_4)) = temp & 0xffffffff;
						break;
					}
					case 8 :
					{
						(*(e.UNSIGNED_INT_8)) = temp;
						break;
					}
				}
				break;
			}
			case SIGNED_INT :
			{
				i8 temp;
				sscanf(str + chars_read, "%ld%n", &temp, &nr);			chars_read += nr;
				switch(tpl_d->element_defs[i].size)
				{
					case 1 :
					{
						(*(e.SIGNED_INT_1)) = temp & 0xff;
						break;
					}
					case 2 :
					{
						(*(e.SIGNED_INT_2)) = temp & 0xffff;
						break;
					}
					case 4 :
					{
						(*(e.SIGNED_INT_4)) = temp & 0xffffffff;
						break;
					}
					case 8 :
					{
						(*(e.SIGNED_INT_8)) = temp;
						break;
					}
				}
				break;
			}
			case FLOATING_NUM :
			{
				switch(tpl_d->element_defs[i].size)
				{
					case 4 :
					{
						f4 temp;
						sscanf(str + chars_read, "%f%n", &temp, &nr); 	chars_read += nr;
						(*(e.FLOATING_NUM_4)) = temp;
						break;
					}
					case 8 :
					{
						f8 temp;
						sscanf(str + chars_read, "%lf%n", &temp, &nr);	chars_read += nr;
						(*(e.FLOATING_NUM_8)) = temp;
						break;
					}
				}
				break;
			}
		}
	}
	sscanf(str + chars_read, "\n%n", &nr); 	chars_read += nr;
	return chars_read;
}