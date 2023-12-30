#include<numeral_element_types.h>

#include<serial_int.h>
#include<float_accesses.h>

int is_numeral_type_element_def(const element_def* ele_d)
{
	switch(ele_d->type)
	{
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
		case BIT_FIELD :
			return 1;
		default :
			return 0;
	}
}

int can_compare_numeral_type_element_def(const element_def* ele_d_1, const element_def* ele_d_2)
{
	switch(ele_d_1->type)
	{
		case UINT :
		{
			switch(ele_d_2->type)
			{
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
				case BIT_FIELD :
					return 1;
				default :
					return 0;
			}
		}
		case INT :
		{
			switch(ele_d_2->type)
			{
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
				case BIT_FIELD :
					return 1;
				default :
					return 0;
			}
		}
		case FLOAT :
		{
			switch(ele_d_2->type)
			{
				case UINT :
				case INT :
				case FLOAT :
				case BIT_FIELD :
					return 1;
				default :
					return 0;
			}
		}
		case LARGE_UINT :
		{
			switch(ele_d_2->type)
			{
				case INT :
				case UINT :
				case LARGE_UINT :
				case BIT_FIELD :
					return 1;
				default :
					return 0;
			}
		}
		case BIT_FIELD :
		{
			switch(ele_d_2->type)
			{
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
				case BIT_FIELD :
					return 1;
				default :
					return 0;
			}
		}
		default :
			return 0;
	}
	return 0;
}

int compare_numeral_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	switch(ele_d_1->type)
	{
		case UINT :
		{
			uint64_t e1_val = deserialize_uint64(e1, ele_d_1->size);
			switch(ele_d_2->type)
			{
				case UINT :
				{
					uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
					return compare_numbers(e1_val, e2_val);
				}
				case INT :
				{
					int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
					return compare_numbers(e1_val, e2_val);
				}
				case FLOAT :
				{
					if(ele_d_2->size == sizeof(float))
					{
						float e2_val = deserialize_float(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else if(ele_d_2->size == sizeof(double))
					{
						double e2_val = deserialize_double(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else if(ele_d_2->size == sizeof(long double))
					{
						long double e2_val = deserialize_long_double(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else
						return -2;
				}
				case LARGE_UINT :
				{
					large_uint e2_val = deserialize_large_uint(e2, ele_d_2->size);
					return compare_large_uint(get_large_uint(e1_val), e2_val);
				}
				default :
					return -2;
			}
		}
		case INT :
		{
			int64_t e1_val = deserialize_int64(e1, ele_d_1->size);
			switch(ele_d_2->type)
			{
				case UINT :
				{
					uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
					return compare_numbers(e1_val, e2_val);
				}
				case INT :
				{
					int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
					return compare_numbers(e1_val, e2_val);
				}
				case FLOAT :
				{
					if(ele_d_2->size == sizeof(float))
					{
						float e2_val = deserialize_float(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else if(ele_d_2->size == sizeof(double))
					{
						double e2_val = deserialize_double(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else if(ele_d_2->size == sizeof(long double))
					{
						long double e2_val = deserialize_long_double(e2);
						return compare_numbers(e1_val, e2_val);
					}
					else
						return -2;
				}
				case LARGE_UINT :
				{
					if(e1_val < 0)
						return -1;
					large_uint e2_val = deserialize_large_uint(e2, ele_d_2->size);
					return compare_large_uint(get_large_uint(e1_val), e2_val);
				}
				default :
					return -2;
			}
		}
		case FLOAT :
		{
			if(ele_d_1->size == sizeof(float))
			{
				float e1_val = deserialize_float(e1);
				switch(ele_d_2->type)
				{
					case UINT :
					{
						uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case INT :
					{
						int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case FLOAT :
					{
						if(ele_d_2->size == sizeof(float))
						{
							float e2_val = deserialize_float(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(double))
						{
							double e2_val = deserialize_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(long double))
						{
							long double e2_val = deserialize_long_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else
							return -2;
					}
					default :
						return -2;
				}
			}
			else if(ele_d_1->size == sizeof(double))
			{
				double e1_val = deserialize_double(e1);
				switch(ele_d_2->type)
				{
					case UINT :
					{
						uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case INT :
					{
						int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case FLOAT :
					{
						if(ele_d_2->size == sizeof(float))
						{
							float e2_val = deserialize_float(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(double))
						{
							double e2_val = deserialize_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(long double))
						{
							long double e2_val = deserialize_long_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else
							return -2;
					}
					default :
						return -2;
				}
			}
			else if(ele_d_1->size == sizeof(long double))
			{
				long double e1_val = deserialize_long_double(e1);
				switch(ele_d_2->type)
				{
					case UINT :
					{
						uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case INT :
					{
						int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
						return compare_numbers(e1_val, e2_val);
					}
					case FLOAT :
					{
						if(ele_d_2->size == sizeof(float))
						{
							float e2_val = deserialize_float(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(double))
						{
							double e2_val = deserialize_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else if(ele_d_2->size == sizeof(long double))
						{
							long double e2_val = deserialize_long_double(e2);
							return compare_numbers(e1_val, e2_val);
						}
						else
							return -2;
					}
					default :
						return -2;
				}
			}
		}
		case LARGE_UINT :
		{
			large_uint e1_val = deserialize_large_uint(e1, ele_d_1->size);
			switch(ele_d_2->type)
			{
				case UINT :
				{
					uint64_t e2_val = deserialize_uint64(e2, ele_d_2->size);
					return compare_large_uint(e1_val, get_large_uint(e2_val));
				}
				case INT :
				{
					int64_t e2_val = deserialize_int64(e2, ele_d_2->size);
					if(e2_val < 0)
						return 1;
					return compare_large_uint(e1_val, get_large_uint(e2_val));
				}
				case LARGE_UINT :
				{
					large_uint e2_val = deserialize_large_uint(e2, ele_d_2->size);
					return compare_large_uint(e1_val, e2_val);
				}
				default :
					return -2;
			}
		}
		default :
			return -2;
	}
}

uint32_t hash_numeral_type_element(const void* e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	return hash_func(e, ele_d->size);
}

void set_numeral_element(void* e, const element_def* ele_d, const user_value* uval)
{
	// ensure that user value is not NULL
	if(is_user_value_NULL(uval))
		return;

	switch(ele_d->type)
	{
		case UINT :
		{
			serialize_uint64(e, ele_d->size, uval->uint_value);
			break;
		}
		case INT :
		{
			serialize_int64(e, ele_d->size, uval->int_value);
			break;
		}
		case FLOAT :
		{
			if(ele_d->size == sizeof(float))
				serialize_float(e, uval->float_value);
			else if(ele_d->size == sizeof(double))
				serialize_double(e, uval->double_value);
			else if(ele_d->size == sizeof(long double))
				serialize_long_double(e, uval->long_double_value);
			break;
		}
		case LARGE_UINT :
		{
			serialize_large_uint(e, ele_d->size, uval->large_uint_value);
			break;
		}
		default :
			break;
	}
}

void set_numeral_element_from_element(void* e, const element_def* ele_d, const void* e_from, const element_def* ele_d_from)
{
	switch(ele_d->type)
	{
		case UINT :
		{
			uint64_t e_new_val = 0;
			switch(ele_d_from->type)
			{
				case UINT :
				{
					uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
					e_new_val = e_from_val;
					break;
				}
				case INT :
				{
					int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
					e_new_val = e_from_val;
					break;
				}
				case FLOAT :
				{
					if(ele_d_from->size == sizeof(float))
					{
						float e_from_val = deserialize_float(e_from);
						e_new_val = e_from_val;
					}
					else if(ele_d_from->size == sizeof(double))
					{
						double e_from_val = deserialize_double(e_from);
						e_new_val = e_from_val;
					}
					else if(ele_d_from->size == sizeof(long double))
					{
						long double e_from_val = deserialize_long_double(e_from);
						e_new_val = e_from_val;
					}
					break;
				}
				case LARGE_UINT :
				{
					large_uint e_from_val = deserialize_large_uint(e_from, ele_d_from->size);
					e_new_val = e_from_val.limbs[0];
					break;
				}
				default :
					break;
			}
			serialize_uint64(e, ele_d->size, e_new_val);
			break;
		}
		case INT :
		{
			int64_t e_new_val = 0;
			switch(ele_d_from->type)
			{
				case UINT :
				{
					uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
					e_new_val = e_from_val;
					break;
				}
				case INT :
				{
					int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
					e_new_val = e_from_val;
					break;
				}
				case FLOAT :
				{
					if(ele_d_from->size == sizeof(float))
					{
						float e_from_val = deserialize_float(e_from);
						e_new_val = e_from_val;
					}
					else if(ele_d_from->size == sizeof(double))
					{
						double e_from_val = deserialize_double(e_from);
						e_new_val = e_from_val;
					}
					else if(ele_d_from->size == sizeof(long double))
					{
						long double e_from_val = deserialize_long_double(e_from);
						e_new_val = e_from_val;
					}
					break;
				}
				case LARGE_UINT :
				{
					large_uint e_from_val = deserialize_large_uint(e_from, ele_d_from->size);
					e_new_val = e_from_val.limbs[0];
					break;
				}
				default :
					break;
			}
			serialize_int64(e, ele_d->size, e_new_val);
			break;
		}
		case FLOAT :
		{
			if(ele_d->size == sizeof(float))
			{
				float e_new_val = 0;
				switch(ele_d_from->type)
				{
					case UINT :
					{
						uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case INT :
					{
						int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case FLOAT :
					{
						if(ele_d_from->size == sizeof(float))
						{
							float e_from_val = deserialize_float(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(double))
						{
							double e_from_val = deserialize_double(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(long double))
						{
							long double e_from_val = deserialize_long_double(e_from);
							e_new_val = e_from_val;
						}
						break;
					}
					default :
						break;
				}
				serialize_float(e, e_new_val);
			}
			else if(ele_d->size == sizeof(double))
			{
				double e_new_val = 0;
				switch(ele_d_from->type)
				{
					case UINT :
					{
						uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case INT :
					{
						int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case FLOAT :
					{
						if(ele_d_from->size == sizeof(float))
						{
							float e_from_val = deserialize_float(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(double))
						{
							double e_from_val = deserialize_double(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(long double))
						{
							long double e_from_val = deserialize_long_double(e_from);
							e_new_val = e_from_val;
						}
						break;
					}
					default :
						break;
				}
				serialize_double(e, e_new_val);
			}
			else if(ele_d->size == sizeof(long double))
			{
				long double e_new_val = 0;
				switch(ele_d_from->type)
				{
					case UINT :
					{
						uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case INT :
					{
						int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
						e_new_val = e_from_val;
						break;
					}
					case FLOAT :
					{
						if(ele_d_from->size == sizeof(float))
						{
							float e_from_val = deserialize_float(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(double))
						{
							double e_from_val = deserialize_double(e_from);
							e_new_val = e_from_val;
						}
						else if(ele_d_from->size == sizeof(long double))
						{
							long double e_from_val = deserialize_long_double(e_from);
							e_new_val = e_from_val;
						}
						break;
					}
					default :
						break;
				}
				serialize_long_double(e, e_new_val);
			}
			break;
		}
		case LARGE_UINT :
		{
			large_uint e_new_val = LARGE_UINT_MIN;
			switch(ele_d_from->type)
			{
				case UINT :
				{
					uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
					e_new_val = get_large_uint(e_from_val);
					break;
				}
				case INT :
				{
					int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
					e_new_val = get_large_uint(e_from_val);
					break;
				}
				case LARGE_UINT :
				{
					e_new_val = deserialize_large_uint(e_from, ele_d_from->size);
					break;
				}
				default :
					break;
			}
			serialize_large_uint(e, ele_d->size, e_new_val);
			break;
		}
		default :
			break;
	}
}

user_value get_value_from_numeral_element(const void* e, const element_def* ele_d)
{
	user_value uval = {};
	switch(ele_d->type)
	{
		case UINT :
		{
			uval.uint_value = deserialize_uint64(e, ele_d->size);
			break;
		}
		case INT :
		{
			uval.int_value = deserialize_int64(e, ele_d->size);
			break;
		}
		case FLOAT :
		{
			if(ele_d->size == sizeof(float))
				uval.float_value = deserialize_float(e);
			else if(ele_d->size == sizeof(double))
				uval.double_value = deserialize_double(e);
			else if(ele_d->size == sizeof(long double))
				uval.long_double_value = deserialize_long_double(e);
		}
		case LARGE_UINT :
		{
			uval.large_uint_value = deserialize_large_uint(e, ele_d->size);
			break;
		}
		default :
			break;
	}
	return uval;
}

user_value get_MIN_value_for_numeral_element_def(const element_def* ele_d)
{
	user_value uval = {};
	switch(ele_d->type)
	{
		case UINT :
		{
			uval.uint_value = get_UINT64_MIN(ele_d->size);
			break;
		}
		case INT :
		{
			uval.int_value = get_INT64_MIN(ele_d->size);
			break;
		}
		case FLOAT :
		{
			if(ele_d->size == sizeof(float))
				uval.float_value = get_FLOAT_MIN();
			else if(ele_d->size == sizeof(double))
				uval.double_value = get_DOUBLE_MIN();
			else if(ele_d->size == sizeof(long double))
				uval.long_double_value = get_LONG_DOUBLE_MIN();
			break;
		}
		case LARGE_UINT :
		{
			uval.large_uint_value = LARGE_UINT_MIN;
			break;
		}
		case BIT_FIELD :
		{
			uval.bit_field_value = 0;
			break;
		}
		default :
			break;
	}
	return uval;
}

user_value get_MAX_value_for_numeral_element_def(const element_def* ele_d)
{
	user_value uval = {};
	switch(ele_d->type)
	{
		case UINT :
		{
			uval.uint_value = get_UINT64_MAX(ele_d->size);
			break;
		}
		case INT :
		{
			uval.int_value = get_INT64_MAX(ele_d->size);
			break;
		}
		case FLOAT :
		{
			if(ele_d->size == sizeof(float))
				uval.float_value = get_FLOAT_MAX();
			else if(ele_d->size == sizeof(double))
				uval.double_value = get_DOUBLE_MAX();
			else if(ele_d->size == sizeof(long double))
				uval.long_double_value = get_LONG_DOUBLE_MAX();
			break;
		}
		case LARGE_UINT :
		{
			uval.large_uint_value = LARGE_UINT_MAX;
			break;
		}
		case BIT_FIELD :
		{
			uval.bit_field_value = get_UINT64_MAX(ele_d->size);
			break;
		}
		default :
			break;
	}
	return uval;
}