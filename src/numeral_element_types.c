#include<numeral_element_types.h>

#include<int_accesses.h>

int is_numeral_type_element_def(const element_def* ele_d)
{
	switch(ele_d->type)
	{
		case UINT :
		case INT :
		case FLOAT :
			return 1;
		default :
			return 0;
	}
}

int compare_numeral_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	switch(ele_d_1->type)
	{
		case UINT :
		{
			uint64_t e1_val = read_uint64(e1, ele_d_1->size);
			switch(ele_d_2->type)
			{
				case UINT :
				{
					uint64_t e2_val = read_uint64(e2, ele_d_2->size);
					return compare(e1_val, e2_val);
				}
				case INT :
				{
					int64_t e2_val = read_int64(e2, ele_d_2->size);
					return compare(e1_val, e2_val);
				}
				case FLOAT :
				{
					switch(ele_d_2->size)
					{
						case 4 :
						{
							float e2_val = read_float(e2);
							return compare(e1_val, e2_val);
						}
						case 8 :
						{
							double e2_val = read_double(e2);
							return compare(e1_val, e2_val);
						}
					}
				}
				default :
					break;
			}
		}
		case INT :
		{
			int64_t e1_val = read_int64(e1, ele_d_1->size);
			switch(ele_d_2->type)
			{
				case UINT :
				{
					uint64_t e2_val = read_uint64(e2, ele_d_2->size);
					return compare(e1_val, e2_val);
				}
				case INT :
				{
					int64_t e2_val = read_int64(e2, ele_d_2->size);
					return compare(e1_val, e2_val);
				}
				case FLOAT :
				{
					switch(ele_d_2->size)
					{
						case 4 :
						{
							float e2_val = read_float(e2);
							return compare(e1_val, e2_val);
						}
						case 8 :
						{
							double e2_val = read_double(e2);
							return compare(e1_val, e2_val);
						}
					}
				}
				default :
					break;
			}
		}
		case FLOAT :
		{
			switch(ele_d_1->size)
			{
				case 4 :
				{
					float e1_val = read_float(e1);
					switch(ele_d_2->type)
					{
						case UINT :
						{
							uint64_t e2_val = read_uint64(e2, ele_d_2->size);
							return compare(e1_val, e2_val);
						}
						case INT :
						{
							int64_t e2_val = read_int64(e2, ele_d_2->size);
							return compare(e1_val, e2_val);
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 :
								{
									float e2_val = read_float(e2);
									return compare(e1_val, e2_val);
								}
								case 8 :
								{
									double e2_val = read_double(e2);
									return compare(e1_val, e2_val);
								}
							}
						}
						default :
							break;
					}
				}
				case 8 :
				{
					double e1_val = read_double(e1);
					switch(ele_d_2->type)
					{
						case UINT :
						{
							uint64_t e2_val = read_uint64(e2, ele_d_2->size);
							return compare(e1_val, e2_val);
						}
						case INT :
						{
							int64_t e2_val = read_int64(e2, ele_d_2->size);
							return compare(e1_val, e2_val);
						}
						case FLOAT :
						{
							switch(ele_d_2->size)
							{
								case 4 :
								{
									float e2_val = read_float(e2);
									return compare(e1_val, e2_val);
								}
								case 8 :
								{
									double e2_val = read_double(e2);
									return compare(e1_val, e2_val);
								}
							}
						}
						default :
							break;
					}
				}
			}
		}
		default :
			break;
	}
	return -2;
}

void set_numeral_element(void* e, const element_def* ele_d, const user_value* uval)
{
	switch(ele_d->type)
	{
		case UINT :
		{
			write_uint64(e, ele_d->size, uval->uint_value);
			break;
		}
		case INT :
		{
			write_int64(e, ele_d->size, uval->int_value);
			break;
		}
		case FLOAT :
		{
			switch(ele_d->size)
			{
				case 4 :
				{
					write_float(e, uval->float_value);
					break;
				}
				case 8 :
				{
					write_double(e, uval->double_value);
					break;
				}
			}
		}
		default :
			break;
	}
}