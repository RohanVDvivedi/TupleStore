#include<tuple.h>

element seek_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d)
{
	element e;
	e.GENERIC = (tupl + (tpl_d->element_defs[column_no].offset));
	return e;
}

void copy_to_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value)
{
	void* vp = seek_cell(tpl_d, tupl, column_no).GENERIC;
	if(tpl_d->element_defs[column_no].endian == CPU_ENDIAN
		|| tpl_d->element_defs[column_no].type == CHAR_STRING
		|| tpl_d->element_defs[column_no].size == 1)
	{
		memcpy(vp, value, tpl_d->element_defs[column_no].size);
	}
	else
	{
		switch(tpl_d->element_defs[column_no].size)
		{
			case 2 :
			{
				*((u2*)vp) = bswap_16(*((u2*)value));
				break;
			}
			case 4 :
			{
				*((u4*)vp) = bswap_32(*((u4*)value));
				break;
			}
			case 8 :
			{
				*((u8*)vp) = bswap_64(*((u8*)value));
				break;
			}
		}
	}
}

void copy_from_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value)
{
	void* vp = seek_cell(tpl_d, tupl, column_no).GENERIC;
	if(tpl_d->element_defs[column_no].endian == CPU_ENDIAN
		|| tpl_d->element_defs[column_no].type == CHAR_STRING
		|| tpl_d->element_defs[column_no].size == 1)
	{
		memcpy(value, vp, tpl_d->element_defs[column_no].size);
	}
	else
	{
		switch(tpl_d->element_defs[column_no].size)
		{
			case 2 :
			{
				*((u2*)value) = bswap_16(*((u2*)vp));
				break;
			}
			case 4 :
			{
				*((u4*)value) = bswap_32(*((u4*)vp));
				break;
			}
			case 8 :
			{
				*((u8*)value) = bswap_64(*((u8*)vp));
				break;
			}
		}
	}
}

int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d)
{
	return memcmp(tup1, tup2, tpl_d->key_size);
}

void swap_tuples(void* tup1, void* tup2, tuple_def* tpl_d)
{
	void* temp_tupl = alloca(tpl_d->size);
	memcpy(temp_tupl, tup1, tpl_d->size);
	memcpy(temp_tupl, tup1, tpl_d->size);
	memcpy(temp_tupl, tup1, tpl_d->size);
}

void print_tuple(void* tup, tuple_def* tpl_d)
{
	for(uint16_t i = 0; i < tpl_d->element_count; i++)
	{
		if(i)
			printf(", ");

		switch(tpl_d->element_defs[column_no].type)
		{
			case CHAR_STRING :
			{
				printf("%s", seek_cell(tpl_d, tup, i).CHAR_STRING);
				break;
			}
			case UNSIGNED_INT :
			{
				break;
			}
			case SIGNED_INT :
			{
				break;
			}
			case FLOAT_NUM :
			{
				break;
			}
		}
	}
	printf("\n");
}