#include<primitive_numeral_types.h>

#include<serial_int.h>
#include<float_accesses.h>
#include<bitmap.h>

int is_primitive_numeral_type_info(const data_type_info* dti)
{
	switch(dti->type)
	{
		case BIT_FIELD :
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
			return 1;
		default :
			return 0;
	}
}

int can_compare_primitive_numeral_type_infos(const data_type_info* dti_1, const data_type_info* dti_2)
{
	switch(dti_1->type)
	{
		case BIT_FIELD :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
					return 1;
				default :
					return 0;
			}
		}
		case UINT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
					return 1;
				default :
					return 0;
			}
		}
		case INT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case FLOAT :
				case LARGE_UINT :
					return 1;
				default :
					return 0;
			}
		}
		case FLOAT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case FLOAT :
					return 1;
				default :
					return 0;
			}
		}
		case LARGE_UINT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case LARGE_UINT :
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

int compare_primitive_numeral_type(const user_value* e1, const data_type_info* dti_1, const user_value* e2, const data_type_info* dti_2);
{
	if(!can_compare_primitive_numeral_type_infos(dti_1, dti_2))
		return -2;

	if(is_user_value_NULL(e1) && is_user_value_NULL(e2))
		return 0;
	else if(is_user_value_NULL(e1) && !is_user_value_NULL(e2))
		return -1;
	else if(!is_user_value_NULL(e1) && is_user_value_NULL(e2))
		return 1;

	switch(dti_1->type)
	{
		case BIT_FIELD :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
					return compare_numbers(e1->bit_field_value, e2->bit_field_value);
				case UINT :
					return compare_numbers(e1->bit_field_value, e2->uint_value);
				case INT :
					return compare_numbers(e1->bit_field_value, e2->int_value);
				case FLOAT :
				{
					if(dti_2->size == sizeof(float))
						return compare_numbers(e1->bit_field_value, e2->float_value);
					else if(dti_2->size == sizeof(double))
						return compare_numbers(e1->bit_field_value, e2->double_value);
					else if(dti_2->size == sizeof(long double))
						return compare_numbers(e1->bit_field_value, e2->long_double_value);
					else
						return -2;
				}
				case LARGE_UINT :
					return compare_uint256(get_uint256(e1->bit_field_value), e2->large_uint_value);
				default :
					return -2;
			}
		}
		case UINT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
					return compare_numbers(e1->uint_value, e2->bit_field_value);
				case UINT :
					return compare_numbers(e1->uint_value, e2->uint_value);
				case INT :
					return compare_numbers(e1->uint_value, e2->int_value);
				case FLOAT :
				{
					if(dti_2->size == sizeof(float))
						return compare_numbers(e1->uint_value, e2->float_value);
					else if(dti_2->size == sizeof(double))
						return compare_numbers(e1->uint_value, e2->double_value);
					else if(dti_2->size == sizeof(long double))
						return compare_numbers(e1->uint_value, e2->long_double_value);
					else
						return -2;
				}
				case LARGE_UINT :
					return compare_uint256(get_uint256(e1->uint_value), e2->large_uint_value);
				default :
					return -2;
			}
		}
		case INT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
					return compare_numbers(e1->int_value, e2->bit_field_value);
				case UINT :
					return compare_numbers(e1->int_value, e2->uint_value);
				case INT :
					return compare_numbers(e1->int_value, e2->int_value);
				case FLOAT :
				{
					if(dti_2->size == sizeof(float))
						return compare_numbers(e1->int_value, e2->float_value);
					else if(dti_2->size == sizeof(double))
						return compare_numbers(e1->int_value, e2->double_value);
					else if(dti_2->size == sizeof(long double))
						return compare_numbers(e1->int_value, e2->long_double_value);
					else
						return -2;
				}
				case LARGE_UINT :
				{
					if(e1->int_value < 0)
						return -1;
					return compare_uint256(get_uint256(e1->int_value), e2->large_uint_value);
				}
				default :
					return -2;
			}
		}
		case FLOAT :
		{
			if(dti_1->size == sizeof(float))
			{
				switch(dti_2->type)
				{
					case BIT_FIELD :
						return compare_numbers(e1->float_value, e2->bit_field_value);
					case UINT :
						return compare_numbers(e1->float_value, e2->uint_value);
					case INT :
						return compare_numbers(e1->float_value, e2->int_value);
					case FLOAT :
					{
						if(dti_2->size == sizeof(float))
							return compare_numbers(e1->float_value, e2->float_value);
						else if(dti_2->size == sizeof(double))
							return compare_numbers(e1->float_value, e2->double_value);
						else if(dti_2->size == sizeof(long double))
							return compare_numbers(e1->float_value, e2->long_double_value);
						else
							return -2;
					}
					default :
						return -2;
				}
			}
			else if(dti_1->size == sizeof(double))
			{
				switch(dti_2->type)
				{
					case BIT_FIELD :
						return compare_numbers(e1->double_value, e2->bit_field_value);
					case UINT :
						return compare_numbers(e1->double_value, e2->uint_value);
					case INT :
						return compare_numbers(e1->double_value, e2->int_value);
					case FLOAT :
					{
						if(dti_2->size == sizeof(float))
							return compare_numbers(e1->double_value, e2->float_value);
						else if(dti_2->size == sizeof(double))
							return compare_numbers(e1->double_value, e2->double_value);
						else if(dti_2->size == sizeof(long double))
							return compare_numbers(e1->double_value, e2->long_double_value);
						else
							return -2;
					}
					default :
						return -2;
				}
			}
			else if(dti_1->size == sizeof(long double))
			{
				switch(dti_2->type)
				{
					case BIT_FIELD :
						return compare_numbers(e1->long_double_value, e2->bit_field_value);
					case UINT :
						return compare_numbers(e1->long_double_value, e2->uint_value);
					case INT :
						return compare_numbers(e1->long_double_value, e2->int_value);
					case FLOAT :
					{
						if(dti_2->size == sizeof(float))
							return compare_numbers(e1->long_double_value, e2->float_value);
						else if(dti_2->size == sizeof(double))
							return compare_numbers(e1->long_double_value, e2->double_value);
						else if(dti_2->size == sizeof(long double))
							return compare_numbers(e1->long_double_value, e2->long_double_value);
						else
							return -2;
					}
					default :
						return -2;
				}
			}
			else
				return -2;
		}
		case LARGE_UINT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
					return compare_uint256(e1->large_uint_value, get_uint256(e2->bit_field_value));
				case UINT :
					return compare_uint256(e1->large_uint_value, get_uint256(e2->uint_value));
				case INT :
				{
					if(e2->int_value < 0)
						return 1;
					return compare_uint256(e1->large_uint_value, get_uint256(e2->int_value));
				}
				case LARGE_UINT :
					return compare_uint256(e1->large_uint_value, e2->large_uint_value);
				default :
					return -2;
			}
		}
		default :
			return -2;
	}
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
			serialize_uint256(e, ele_d->size, uval->large_uint_value);
			break;
		}
		case BIT_FIELD :
		{
			set_bits(e, ele_d->bit_offset, ele_d->bit_offset + ele_d->size - 1, uval->bit_field_value);
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
					uint256 e_from_val = deserialize_uint256(e_from, ele_d_from->size);
					e_new_val = e_from_val.limbs[0];
					break;
				}
				case BIT_FIELD :
				{
					uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
					e_new_val = e_from_val;
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
					uint256 e_from_val = deserialize_uint256(e_from, ele_d_from->size);
					e_new_val = e_from_val.limbs[0];
					break;
				}
				case BIT_FIELD :
				{
					uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
					e_new_val = e_from_val;
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
					case BIT_FIELD :
					{
						uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
						e_new_val = e_from_val;
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
					case BIT_FIELD :
					{
						uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
						e_new_val = e_from_val;
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
					case BIT_FIELD :
					{
						uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
						e_new_val = e_from_val;
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
			uint256 e_new_val = get_0_uint256();
			switch(ele_d_from->type)
			{
				case UINT :
				{
					uint64_t e_from_val = deserialize_uint64(e_from, ele_d_from->size);
					e_new_val = get_uint256(e_from_val);
					break;
				}
				case INT :
				{
					int64_t e_from_val = deserialize_int64(e_from, ele_d_from->size);
					e_new_val = get_uint256(e_from_val);
					break;
				}
				case LARGE_UINT :
				{
					e_new_val = deserialize_uint256(e_from, ele_d_from->size);
					break;
				}
				case BIT_FIELD :
				{
					uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
					e_new_val = get_uint256(e_from_val);
					break;
				}
				default :
					break;
			}
			serialize_uint256(e, ele_d->size, e_new_val);
			break;
		}
		case BIT_FIELD :
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
					uint256 e_from_val = deserialize_uint256(e_from, ele_d_from->size);
					e_new_val = e_from_val.limbs[0];
					break;
				}
				case BIT_FIELD :
				{
					uint64_t e_from_val = get_bits(e_from, ele_d_from->bit_offset, ele_d_from->bit_offset + ele_d_from->size - 1);
					e_new_val = e_from_val;
					break;
				}
				default :
					break;
			}
			set_bits(e, ele_d->bit_offset, ele_d->bit_offset + ele_d->size - 1, e_new_val);
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
			uval.large_uint_value = deserialize_uint256(e, ele_d->size);
			break;
		}
		case BIT_FIELD :
		{
			uval.bit_field_value = get_bits(e, ele_d->bit_offset, ele_d->bit_offset + ele_d->size - 1);
			break;
		}
		default :
			break;
	}
	return uval;
}

user_value get_MIN_value_for_primitive_numeral_type_info(const data_type_info* dti)
{
	user_value uval = {};
	switch(dti->type)
	{
		case UINT :
		{
			uval.uint_value = get_UINT64_MIN(dti->size);
			break;
		}
		case INT :
		{
			uval.int_value = get_INT64_MIN(dti->size);
			break;
		}
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				uval.float_value = get_FLOAT_MIN();
			else if(dti->size == sizeof(double))
				uval.double_value = get_DOUBLE_MIN();
			else if(dti->size == sizeof(long double))
				uval.long_double_value = get_LONG_DOUBLE_MIN();
			break;
		}
		case LARGE_UINT :
		{
			uval.large_uint_value = get_min_uint256();
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

user_value get_MAX_value_for_primitive_numeral_type_info(const data_type_info* dti)
{
	user_value uval = {};
	switch(dti->type)
	{
		case UINT :
		{
			uval.uint_value = get_UINT64_MAX(dti->size);
			break;
		}
		case INT :
		{
			uval.int_value = get_INT64_MAX(dti->size);
			break;
		}
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				uval.float_value = get_FLOAT_MAX();
			else if(dti->size == sizeof(double))
				uval.double_value = get_DOUBLE_MAX();
			else if(dti->size == sizeof(long double))
				uval.long_double_value = get_LONG_DOUBLE_MAX();
			break;
		}
		case LARGE_UINT :
		{
			// returns a uint256, with least significant (dti->size * CHAR_BIT) bits set to 1
			uval.large_uint_value = get_bitmask_lower_n_bits_set_uint256(dti->size * CHAR_BIT);
			break;
		}
		case BIT_FIELD :
		{
			uval.bit_field_value = ((dti->bit_field_size == 64) ? UINT64_MAX : ((UINT64_C(1) << dti->bit_field_size) - 1));
			break;
		}
		default :
			break;
	}
	return uval;
}