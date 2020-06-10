#include<tuple.h>

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
	return memcmp(tup1, tup2, tpl_d->key_size);
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
						chars_written += sprintf(str + chars_written, "%u",  (*(e.UNSIGNED_INT_1)) & 0xff);
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%u",  (*(e.UNSIGNED_INT_2)) & 0xffff);
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%u", (*(e.UNSIGNED_INT_4)) & 0xffffffff);
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
						chars_written += sprintf(str + chars_written, "%d", (*(e.SIGNED_INT_1)) & 0xff);
						break;
					}
					case 2 :
					{
						chars_written += sprintf(str + chars_written, "%d", (*(e.SIGNED_INT_2)) & 0xffff);
						break;
					}
					case 4 :
					{
						chars_written += sprintf(str + chars_written, "%d", (*(e.SIGNED_INT_4)) & 0xffffffff);
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

void sscan_tuple(char* str, void* tup, tuple_def* tpl_d)
{

}