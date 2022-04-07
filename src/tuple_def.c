#include<tuple_def.h>

#include<string.h>

#include<bitmap.h>

#include<page_layout_util.h>

char type_as_string[][16] = {
								"UINT",
								"INT",
								"FLOAT",
								"STRING",
								"BLOB",
								"VAR_STRING",
								"VAR_BLOB"
							};

static int is_variable_sized_element_type(element_type ele_type)
{
	return ele_type == VAR_STRING || ele_type == VAR_BLOB;
}

static int is_size_allowed_for_fixed_sized_type(element_type ele_type, uint32_t size)
{
	switch(ele_type)
	{
		case UINT :
		case INT :
			return (size == 1) || (size == 2) || (size == 4) || (size == 8);
		case FLOAT :
			return (size == 4) || (size == 8);
		case STRING :
		case BLOB :	// STRING and BLOB can be of any size
			return 1;
		default :
			return 0;
	}
}

static int is_prefix_size_allowed_for_variable_sized_type(element_type ele_type, uint32_t prefix_size)
{
	switch(ele_type)
	{
		case VAR_STRING :
		case VAR_BLOB :
			return (prefix_size == 1) || (prefix_size == 2) || (prefix_size == 4);
		default :
			return 0;
	}
}

int init_element_def(element_def* element_d, const char* name, element_type ele_type, uint32_t size_OR_prefix_size)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	if(is_variable_sized_element_type(ele_type))
	{
		if(!is_prefix_size_allowed_for_variable_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->size_specifier_prefix_size = size_OR_prefix_size;

		// set the byte_offset_to_byte_offset to 0
		// we don't know it yet
		element_d->byte_offset_to_byte_offset = 0;
	}
	else
	{
		if(!is_size_allowed_for_fixed_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->size = size_OR_prefix_size;

		// set the byte_offset to 0
		// we don't know it yet
		element_d->byte_offset = 0;
	}

	// set type to the ele_type as in parameter
	element_d->type = ele_type;

	// add name to this element definition
	strncpy(element_d->name, name, 63);
	element_d->name[63] = '\0';

	return 1;
}

int is_variable_sized_element_def(const element_def* element_d)
{
	return is_variable_sized_element_type(element_d->type);
}

int is_fixed_sized_element_def(const element_def* element_d)
{
	return !is_variable_sized_element_def(element_d);
}

uint32_t get_element_size(element e, const element_def* ele_d)
{
	if(is_fixed_sized_element_def(ele_d))
		return ele_d->size;
	else if(ele_d->type == VAR_STRING)
	{
		switch(ele_d->size_specifier_prefix_size)
		{
			case 1 :
				return 1 + e.VAR_STRING_1->size;
			case 2 :
				return 2 + e.VAR_STRING_2->size;
			case 4 :
				return 4 + e.VAR_STRING_4->size;
			default :
				return 0;
		}
	}
	else if(ele_d->type == VAR_BLOB)
	{
		switch(ele_d->size_specifier_prefix_size)
		{
			case 1 :
				return 1 + e.VAR_BLOB_1->size;
			case 2 :
				return 2 + e.VAR_BLOB_2->size;
			case 4 :
				return 4 + e.VAR_BLOB_4->size;
			default :
				return 0;
		}
	}
	return 0;
}

int can_compare_element_defs(const element_def* ele_d_1, const element_def* ele_d_2)
{
	if((ele_d_1->type == UINT || ele_d_1->type == INT || ele_d_1->type == FLOAT)
	 && (ele_d_2->type == UINT || ele_d_2->type == INT || ele_d_2->type == FLOAT))
		return 1;
	else if((ele_d_1->type == STRING || ele_d_1->type == VAR_STRING)
	 && (ele_d_2->type == STRING || ele_d_2->type == VAR_STRING))
		return 1;
	else if((ele_d_1->type == BLOB || ele_d_1->type == VAR_BLOB)
	 && (ele_d_2->type == BLOB || ele_d_2->type == VAR_BLOB))
		return 1;
	return 0;
}

#define compare(a,b)	( ((a)>(b)) ? 1 : (((a)<(b)) ? (-1) : 0 ) )
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

static int compare_string_types(char* s1, uint32_t s1_max_len, char* s2, uint32_t s2_max_len)
{
	// find actual length of both the strings
	uint32_t s1_len = strnlen(s1, s1_max_len);
	uint32_t s2_len = strnlen(s2, s2_max_len);

	// find min of their lengths
	uint32_t min_len = min(s1_len, s2_len);

	// compare only their min_len number of characters
	int compare = strncmp(s1, s2, min_len);

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else if((compare == 0) && (s1_len != s2_len))
	{
		// in dictionary ordering if 1 string is a prefix of the other
		// then the larger string comes latter in the order
		if(s1_len > s2_len)
			compare = 1;
		else if(s1_len < s2_len)
			compare = -1;
	}

	return compare;
}

static int compare_blob_types(void* b1, uint32_t b1_len, void* b2, uint32_t b2_len)
{
	// find min of their lengths
	uint32_t min_len = min(b1_len, b2_len);

	// compare only their min_len number of bytes
	int compare = memcmp(b1, b2, min_len);

	if(compare > 0)
		compare = 1;
	else if(compare < 0)
		compare = -1;
	else if((compare == 0) && (b1_len != b2_len))
	{
		// in dictionary ordering if 1 string is a prefix of the other
		// then the larger string comes latter in the order
		if(b1_len > b2_len)
			compare = -1;
		else if(b1_len > b2_len)
			compare = 1;
	}

	return compare;
}

int compare_elements(element e1, const element_def* ele_d_1, element e2, const element_def* ele_d_2)
{
	switch(ele_d_1->type)
	{
		case UINT :
		{
			switch(ele_d_1->size)
			{
				case 1 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_1, *e2.UINT_1); }
								case 2 : { return compare(*e1.UINT_1, *e2.UINT_2); }
								case 4 : { return compare(*e1.UINT_1, *e2.UINT_4); }
								case 8 : { return compare(*e1.UINT_1, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_1, *e2.INT_1); }
								case 2 : { return compare(*e1.UINT_1, *e2.INT_2); }
								case 4 : { return compare(*e1.UINT_1, *e2.INT_4); }
								case 8 : { return compare(*e1.UINT_1, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.UINT_1, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.UINT_1, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_2, *e2.UINT_1); }
								case 2 : { return compare(*e1.UINT_2, *e2.UINT_2); }
								case 4 : { return compare(*e1.UINT_2, *e2.UINT_4); }
								case 8 : { return compare(*e1.UINT_2, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_2, *e2.INT_1); }
								case 2 : { return compare(*e1.UINT_2, *e2.INT_2); }
								case 4 : { return compare(*e1.UINT_2, *e2.INT_4); }
								case 8 : { return compare(*e1.UINT_2, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.UINT_2, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.UINT_2, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_4, *e2.UINT_1); }
								case 2 : { return compare(*e1.UINT_4, *e2.UINT_2); }
								case 4 : { return compare(*e1.UINT_4, *e2.UINT_4); }
								case 8 : { return compare(*e1.UINT_4, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_4, *e2.INT_1); }
								case 2 : { return compare(*e1.UINT_4, *e2.INT_2); }
								case 4 : { return compare(*e1.UINT_4, *e2.INT_4); }
								case 8 : { return compare(*e1.UINT_4, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.UINT_4, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.UINT_4, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_8, *e2.UINT_1); }
								case 2 : { return compare(*e1.UINT_8, *e2.UINT_2); }
								case 4 : { return compare(*e1.UINT_8, *e2.UINT_4); }
								case 8 : { return compare(*e1.UINT_8, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.UINT_8, *e2.INT_1); }
								case 2 : { return compare(*e1.UINT_8, *e2.INT_2); }
								case 4 : { return compare(*e1.UINT_8, *e2.INT_4); }
								case 8 : { return compare(*e1.UINT_8, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.UINT_8, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.UINT_8, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
			}
		}
		case INT :
		{
			switch(ele_d_1->size)
			{
				case 1 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_1, *e2.UINT_1); }
								case 2 : { return compare(*e1.INT_1, *e2.UINT_2); }
								case 4 : { return compare(*e1.INT_1, *e2.UINT_4); }
								case 8 : { return compare(*e1.INT_1, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_1, *e2.INT_1); }
								case 2 : { return compare(*e1.INT_1, *e2.INT_2); }
								case 4 : { return compare(*e1.INT_1, *e2.INT_4); }
								case 8 : { return compare(*e1.INT_1, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.INT_1, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.INT_1, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_2, *e2.UINT_1); }
								case 2 : { return compare(*e1.INT_2, *e2.UINT_2); }
								case 4 : { return compare(*e1.INT_2, *e2.UINT_4); }
								case 8 : { return compare(*e1.INT_2, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_2, *e2.INT_1); }
								case 2 : { return compare(*e1.INT_2, *e2.INT_2); }
								case 4 : { return compare(*e1.INT_2, *e2.INT_4); }
								case 8 : { return compare(*e1.INT_2, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.INT_2, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.INT_2, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_4, *e2.UINT_1); }
								case 2 : { return compare(*e1.INT_4, *e2.UINT_2); }
								case 4 : { return compare(*e1.INT_4, *e2.UINT_4); }
								case 8 : { return compare(*e1.INT_4, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_4, *e2.INT_1); }
								case 2 : { return compare(*e1.INT_4, *e2.INT_2); }
								case 4 : { return compare(*e1.INT_4, *e2.INT_4); }
								case 8 : { return compare(*e1.INT_4, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.INT_4, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.INT_4, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_8, *e2.UINT_1); }
								case 2 : { return compare(*e1.INT_8, *e2.UINT_2); }
								case 4 : { return compare(*e1.INT_8, *e2.UINT_4); }
								case 8 : { return compare(*e1.INT_8, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.INT_8, *e2.INT_1); }
								case 2 : { return compare(*e1.INT_8, *e2.INT_2); }
								case 4 : { return compare(*e1.INT_8, *e2.INT_4); }
								case 8 : { return compare(*e1.INT_8, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.INT_8, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.INT_8, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
			}
		}
		case FLOAT :
		{
			switch(ele_d_1->size)
			{
				case 4 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.FLOAT_4, *e2.UINT_1); }
								case 2 : { return compare(*e1.FLOAT_4, *e2.UINT_2); }
								case 4 : { return compare(*e1.FLOAT_4, *e2.UINT_4); }
								case 8 : { return compare(*e1.FLOAT_4, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.FLOAT_4, *e2.INT_1); }
								case 2 : { return compare(*e1.FLOAT_4, *e2.INT_2); }
								case 4 : { return compare(*e1.FLOAT_4, *e2.INT_4); }
								case 8 : { return compare(*e1.FLOAT_4, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.FLOAT_4, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.FLOAT_4, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
				case 8 :
				{
					switch(ele_d_2->type)
					{
						case UINT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.FLOAT_8, *e2.UINT_1); }
								case 2 : { return compare(*e1.FLOAT_8, *e2.UINT_2); }
								case 4 : { return compare(*e1.FLOAT_8, *e2.UINT_4); }
								case 8 : { return compare(*e1.FLOAT_8, *e2.UINT_8); }
							}
						}
						case INT :
						{
							switch(ele_d_2->size)
							{
								case 1 : { return compare(*e1.FLOAT_8, *e2.INT_1); }
								case 2 : { return compare(*e1.FLOAT_8, *e2.INT_2); }
								case 4 : { return compare(*e1.FLOAT_8, *e2.INT_4); }
								case 8 : { return compare(*e1.FLOAT_8, *e2.INT_8); }
							}
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 : { return compare(*e1.FLOAT_8, *e2.FLOAT_4); }
								case 8 : { return compare(*e1.FLOAT_8, *e2.FLOAT_8); }
							}
						}
						default : {break;}
					}
				}
			}
		}
		case STRING :
		{
			switch(ele_d_2->type)
			{
				case STRING : { return compare_string_types(e1.STRING, ele_d_1->size, e2.STRING, ele_d_2->size);}
				case VAR_STRING :
				{
					switch(ele_d_2->size_specifier_prefix_size)
					{
						case 1 : { return compare_string_types(e1.STRING, ele_d_1->size, e2.VAR_STRING_1->string, e2.VAR_STRING_1->size);}
						case 2 : { return compare_string_types(e1.STRING, ele_d_1->size, e2.VAR_STRING_2->string, e2.VAR_STRING_2->size);}
						case 4 : { return compare_string_types(e1.STRING, ele_d_1->size, e2.VAR_STRING_4->string, e2.VAR_STRING_4->size);}
					}
				}
				default : {break;}
			}
		}
		case VAR_STRING :
		{
			switch(ele_d_1->size_specifier_prefix_size)
			{
				case 1 :
				{
					switch(ele_d_2->type)
					{
						case STRING : { return compare_string_types(e1.VAR_STRING_1->string, e1.VAR_STRING_1->size, e2.STRING, ele_d_2->size);}
						case VAR_STRING :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_string_types(e1.VAR_STRING_1->string, e1.VAR_STRING_1->size, e2.VAR_STRING_1->string, e2.VAR_STRING_1->size);}
								case 2 : { return compare_string_types(e1.VAR_STRING_1->string, e1.VAR_STRING_1->size, e2.VAR_STRING_2->string, e2.VAR_STRING_2->size);}
								case 4 : { return compare_string_types(e1.VAR_STRING_1->string, e1.VAR_STRING_1->size, e2.VAR_STRING_4->string, e2.VAR_STRING_4->size);}
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_2->type)
					{
						case STRING : { return compare_string_types(e1.VAR_STRING_2->string, e1.VAR_STRING_2->size, e2.STRING, ele_d_2->size);}
						case VAR_STRING :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_string_types(e1.VAR_STRING_2->string, e1.VAR_STRING_2->size, e2.VAR_STRING_1->string, e2.VAR_STRING_1->size);}
								case 2 : { return compare_string_types(e1.VAR_STRING_2->string, e1.VAR_STRING_2->size, e2.VAR_STRING_2->string, e2.VAR_STRING_2->size);}
								case 4 : { return compare_string_types(e1.VAR_STRING_2->string, e1.VAR_STRING_2->size, e2.VAR_STRING_4->string, e2.VAR_STRING_4->size);}
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_2->type)
					{
						case STRING : { return compare_string_types(e1.VAR_STRING_4->string, e1.VAR_STRING_4->size, e2.STRING, ele_d_2->size);}
						case VAR_STRING :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_string_types(e1.VAR_STRING_4->string, e1.VAR_STRING_4->size, e2.VAR_STRING_1->string, e2.VAR_STRING_1->size);}
								case 2 : { return compare_string_types(e1.VAR_STRING_4->string, e1.VAR_STRING_4->size, e2.VAR_STRING_2->string, e2.VAR_STRING_2->size);}
								case 4 : { return compare_string_types(e1.VAR_STRING_4->string, e1.VAR_STRING_4->size, e2.VAR_STRING_4->string, e2.VAR_STRING_4->size);}
							}
						}
						default : {break;}
					}
				}
			}
		}
		case BLOB :
		{
			switch(ele_d_2->type)
			{
				case BLOB : { return compare_blob_types(e1.BLOB, ele_d_1->size, e2.BLOB, ele_d_2->size);}
				case VAR_BLOB :
				{
					switch(ele_d_2->size_specifier_prefix_size)
					{
						case 1 : { return compare_blob_types(e1.BLOB, ele_d_1->size, e2.VAR_BLOB_1->blob, e2.VAR_BLOB_1->size);}
						case 2 : { return compare_blob_types(e1.BLOB, ele_d_1->size, e2.VAR_BLOB_2->blob, e2.VAR_BLOB_2->size);}
						case 4 : { return compare_blob_types(e1.BLOB, ele_d_1->size, e2.VAR_BLOB_4->blob, e2.VAR_BLOB_4->size);}
					}
				}
				default : {break;}
			}
		}
		case VAR_BLOB :
		{
			switch(ele_d_1->size_specifier_prefix_size)
			{
				case 1 :
				{
					switch(ele_d_2->type)
					{
						case BLOB : { return compare_blob_types(e1.VAR_BLOB_1->blob, e1.VAR_BLOB_1->size, e2.BLOB, ele_d_2->size);}
						case VAR_BLOB :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_blob_types(e1.VAR_BLOB_1->blob, e1.VAR_BLOB_1->size, e2.VAR_BLOB_1->blob, e2.VAR_BLOB_1->size);}
								case 2 : { return compare_blob_types(e1.VAR_BLOB_1->blob, e1.VAR_BLOB_1->size, e2.VAR_BLOB_2->blob, e2.VAR_BLOB_2->size);}
								case 4 : { return compare_blob_types(e1.VAR_BLOB_1->blob, e1.VAR_BLOB_1->size, e2.VAR_BLOB_4->blob, e2.VAR_BLOB_4->size);}
							}
						}
						default : {break;}
					}
				}
				case 2 :
				{
					switch(ele_d_2->type)
					{
						case BLOB : { return compare_blob_types(e1.VAR_BLOB_2->blob, e1.VAR_BLOB_2->size, e2.BLOB, ele_d_2->size);}
						case VAR_BLOB :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_blob_types(e1.VAR_BLOB_2->blob, e1.VAR_BLOB_2->size, e2.VAR_BLOB_1->blob, e2.VAR_BLOB_1->size);}
								case 2 : { return compare_blob_types(e1.VAR_BLOB_2->blob, e1.VAR_BLOB_2->size, e2.VAR_BLOB_2->blob, e2.VAR_BLOB_2->size);}
								case 4 : { return compare_blob_types(e1.VAR_BLOB_2->blob, e1.VAR_BLOB_2->size, e2.VAR_BLOB_4->blob, e2.VAR_BLOB_4->size);}
							}
						}
						default : {break;}
					}
				}
				case 4 :
				{
					switch(ele_d_2->type)
					{
						case BLOB : { return compare_blob_types(e1.VAR_BLOB_4->blob, e1.VAR_BLOB_4->size, e2.BLOB, ele_d_2->size);}
						case VAR_BLOB :
						{
							switch(ele_d_2->size_specifier_prefix_size)
							{
								case 1 : { return compare_blob_types(e1.VAR_BLOB_4->blob, e1.VAR_BLOB_4->size, e2.VAR_BLOB_1->blob, e2.VAR_BLOB_1->size);}
								case 2 : { return compare_blob_types(e1.VAR_BLOB_4->blob, e1.VAR_BLOB_4->size, e2.VAR_BLOB_2->blob, e2.VAR_BLOB_2->size);}
								case 4 : { return compare_blob_types(e1.VAR_BLOB_4->blob, e1.VAR_BLOB_4->size, e2.VAR_BLOB_4->blob, e2.VAR_BLOB_4->size);}
							}
						}
						default : {break;}
					}
				}
			}
		}
	}

	// a return value of -2 implies in comparable types
	return -2;
}

uint32_t hash_element(element e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	// for a STRING or VAR_STRING type the size is the capacity, not the actual size, 
	// the string may be smaller than the size
	if(ele_d->type == STRING)
		return hash_func(e.STRING, strnlen(e.STRING, get_element_size(e, ele_d)));
	else if(ele_d->type == VAR_STRING)
	{
		switch(ele_d->size_specifier_prefix_size)
		{
			case 1 :
				return hash_func(e.VAR_STRING_1->string, strnlen(e.VAR_STRING_1->string, e.VAR_STRING_1->size));
			case 2 :
				return hash_func(e.VAR_STRING_2->string, strnlen(e.VAR_STRING_2->string, e.VAR_STRING_2->size));
			case 4 :
				return hash_func(e.VAR_STRING_4->string, strnlen(e.VAR_STRING_4->string, e.VAR_STRING_4->size));
		}
	}
	else if(ele_d->type == VAR_BLOB)
	{
		switch(ele_d->size_specifier_prefix_size)
		{
			case 1 :
				return hash_func(e.VAR_BLOB_1->blob, e.VAR_BLOB_1->size);
			case 2 :
				return hash_func(e.VAR_BLOB_2->blob, e.VAR_BLOB_2->size);
			case 4 :
				return hash_func(e.VAR_BLOB_4->blob, e.VAR_BLOB_4->size);
		}
	}
	else
		return hash_func(e.BLOB, get_element_size(e, ele_d));

	return 0;
}

int init_tuple_def(tuple_def* tuple_d, const char* name)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	tuple_d->is_variable_sized = 0;
	tuple_d->size = 0;
	tuple_d->byte_offset_to_is_null_bitmap = 0;
	tuple_d->element_count = 0;

	// copy name
	strncpy(tuple_d->name, name, 63);
	tuple_d->name[63] = '\0';

	return 1;
}

int insert_element_def(tuple_def* tuple_d, const char* name, element_type ele_type, uint32_t element_size_OR_prefix_size)
{
	// if an element definition by the name already exists then we fail an insertion
	if(get_element_def_id_by_name(tuple_d, name) != NOT_FOUND)
		return 0;

	// attempt initializing the ith element def
	element_def* new_element_def = tuple_d->element_defs + tuple_d->element_count;
	if(!init_element_def(new_element_def, name, ele_type, element_size_OR_prefix_size))
		return 0;

	tuple_d->element_count++;
	return 1;
}

int insert_copy_of_element_def(tuple_def* tuple_d, const char* name, const tuple_def* tuple_d_copy_from, uint32_t element_def_id)
{
	// element_def_id is out of bounds
	if(element_def_id >= tuple_d_copy_from->element_count)
		return 0;

	if(name == NULL)
		name = tuple_d_copy_from->element_defs[element_def_id].name;

	if(is_fixed_sized_element_def(tuple_d_copy_from->element_defs + element_def_id))
		return insert_element_def(tuple_d, name, tuple_d_copy_from->element_defs[element_def_id].type, tuple_d_copy_from->element_defs[element_def_id].size);
	else
		return insert_element_def(tuple_d, name, tuple_d_copy_from->element_defs[element_def_id].type, tuple_d_copy_from->element_defs[element_def_id].size_specifier_prefix_size);
}

void finalize_tuple_def(tuple_def* tuple_d, uint32_t max_tuple_size)
{
	// calcuate size required to store offsets on a page that is of size max_tuple_size
	uint32_t size_of_offsets = get_value_size_on_page(max_tuple_size);

	tuple_d->size = 0;
	tuple_d->is_variable_sized = 0;

	tuple_d->byte_offset_to_is_null_bitmap = 0;
	tuple_d->size += bitmap_size_in_bytes(tuple_d->element_count);

	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
		{
			tuple_d->is_variable_sized = 1;
			tuple_d->element_defs[i].byte_offset_to_byte_offset = tuple_d->min_size;
			tuple_d->min_size += size_of_offsets;
		}
		else
		{
			tuple_d->element_defs[i].byte_offset = tuple_d->size;
			tuple_d->size += tuple_d->element_defs[i].size;
		}
	}

	if(!tuple_d->is_variable_sized)
		return;

	// if the tuple_def is variable sized we move everything back by the size (in bytes) required to store the size of the tuple
	tuple_d->min_size += size_of_offsets;
	tuple_d->byte_offset_to_is_null_bitmap = size_of_offsets;
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
			tuple_d->element_defs[i].byte_offset_to_byte_offset += size_of_offsets;
		else
			tuple_d->element_defs[i].byte_offset += size_of_offsets;
	}

}

int is_empty_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->element_count == 0;
}

int is_fixed_sized_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->is_variable_sized == 0;
}

int is_variable_sized_tuple_def(const tuple_def* tuple_d)
{
	return !is_fixed_sized_tuple_def(tuple_d);
}

uint32_t get_minimum_tuple_size(const tuple_def* tuple_d)
{
	if(is_fixed_sized_tuple_def(tuple_d))
		return tuple_d->size;
	else
		return tuple_d->min_size;
}

uint32_t get_element_def_id_by_name(const tuple_def* tuple_d, const char* name)
{
	// if the name size is more than or equal to 64 we quit
	uint32_t name_size = strnlen(name, 64);
	if(name_size == 64)
		return NOT_FOUND;

	// we do a linear search here, it is not optimal
	// it should have an associative map, but I dont want to clutter the implementation of tuple def any more 
	// with any more complex data structures
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(0 == strcmp(tuple_d->element_defs[i].name, name))
			return i;
	}

	return NOT_FOUND;
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t \"%s\" of type : %s\n", element_d->name, type_as_string[element_d->type]);
	if(is_variable_sized_element_def(element_d))
	{
		printf("\t\t\t size_specifier_prefix_size : %u\n", element_d->size_specifier_prefix_size);
		printf("\t\t\t byte_offset_to_byte_offset : %u\n", element_d->byte_offset_to_byte_offset);
	}
	else
	{
		printf("\t\t\t size : %u\n", element_d->size);
		printf("\t\t\t byte_offset : %u\n", element_d->byte_offset);
	}
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition for \"%s\" : \n", tuple_d->name);
	printf("is_variable_sized : %u\n", tuple_d->is_variable_sized);
	if(is_variable_sized_tuple_def(tuple_d))
		printf("\t min_size : %u\n", tuple_d->min_size);
	else
		printf("\t size : %u\n", tuple_d->size);
	printf("\t byte_offset_to_is_null_bitmap : %u\n", tuple_d->byte_offset_to_is_null_bitmap);
	printf("\t element_count : %u\n", tuple_d->element_count);
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\t Column : %u\n", i);
		print_element_def((tuple_d->element_defs) + i);
	}
}