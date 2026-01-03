#include<tuplestore/primitive_numeral_types.h>

#include<serint/serial_int.h>
#include<tuplestore/float_accesses.h>
#include<cutlery/bitmap.h>

int is_primitive_numeral_type_info(const data_type_info* dti)
{
	switch(dti->type)
	{
		case BIT_FIELD :
		case UINT :
		case INT :
		case FLOAT :
		case LARGE_UINT :
		case LARGE_INT :
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
				case LARGE_INT :
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
				case LARGE_INT :
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
				case LARGE_INT :
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
				case LARGE_INT :
					return 1;
				default :
					return 0;
			}
		}
		case LARGE_INT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
				case UINT :
				case INT :
				case LARGE_UINT :
				case LARGE_INT :
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

int compare_primitive_numeral_type(const user_value* e1, const data_type_info* dti_1, const user_value* e2, const data_type_info* dti_2)
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
					else
						return -2;
				}
				case LARGE_UINT :
					return compare_uint256(get_uint256(e1->bit_field_value), e2->large_uint_value);
				case LARGE_INT :
					return -compare_int256_uint256(e2->large_int_value, get_uint256(e1->bit_field_value));
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
					else
						return -2;
				}
				case LARGE_UINT :
					return compare_uint256(get_uint256(e1->uint_value), e2->large_uint_value);
				case LARGE_INT :
					return -compare_int256_uint256(e2->large_int_value, get_uint256(e1->uint_value));
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
					else
						return -2;
				}
				case LARGE_UINT :
					return compare_int256_uint256(get_int256(e1->int_value), e2->large_uint_value);
				case LARGE_INT :
					return compare_int256(get_int256(e1->int_value), e2->large_int_value);
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
					return -compare_int256_uint256(get_int256(e2->int_value), e1->large_uint_value);
				case LARGE_UINT :
					return compare_uint256(e1->large_uint_value, e2->large_uint_value);
				case LARGE_INT :
					return -compare_int256_uint256(e2->large_int_value, e1->large_uint_value);
				default :
					return -2;
			}
		}
		case LARGE_INT :
		{
			switch(dti_2->type)
			{
				case BIT_FIELD :
					return compare_int256_uint256(e1->large_int_value, get_uint256(e2->bit_field_value));
				case UINT :
					return compare_int256_uint256(e1->large_int_value, get_uint256(e2->uint_value));
				case INT :
					return compare_int256(e1->large_int_value, get_int256(e2->int_value));
				case LARGE_UINT :
					return compare_int256_uint256(e1->large_int_value, e2->large_uint_value);
				case LARGE_INT :
					return compare_int256(e2->large_int_value, e1->large_int_value);
				default :
					return -2;
			}
		}
		default :
			return -2;
	}
}

int compare_primitive_numeral_type2(const user_value* e1, const user_value* e2, const data_type_info* dti)
{
	if(is_user_value_NULL(e1) && is_user_value_NULL(e2))
		return 0;
	else if(is_user_value_NULL(e1) && !is_user_value_NULL(e2))
		return -1;
	else if(!is_user_value_NULL(e1) && is_user_value_NULL(e2))
		return 1;

	switch(dti->type)
	{
		case BIT_FIELD :
			return compare_numbers(e1->bit_field_value, e2->bit_field_value);
		case UINT :
			return compare_numbers(e1->uint_value, e2->uint_value);
		case INT :
			return compare_numbers(e1->int_value, e2->int_value);
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				return compare_numbers(e1->float_value, e2->float_value);
			else if(dti->size == sizeof(double))
				return compare_numbers(e1->double_value, e2->double_value);
		}
		case LARGE_UINT :
			return compare_uint256(e1->large_uint_value, e2->large_uint_value);
		case LARGE_INT :
			return compare_int256(e1->large_int_value, e2->large_int_value);
		default :
			return -2;
	}
}

int type_cast_primitive_numeral_type(user_value* e, const data_type_info* dti, const user_value* e_from, const data_type_info* dti_from)
{
	// if e_from is NULL, return 
	if(is_user_value_NULL(e_from))
	{
		if(is_nullable_type_info(dti))
			(*e) = (*NULL_USER_VALUE);
		else
			(*e) = (*EMPTY_USER_VALUE);
		return 1;
	}

	// now we are sure that e_from is not NULL

	switch(dti->type)
	{
		case BIT_FIELD :
		{
			switch(dti_from->type)
			{
				case BIT_FIELD :
				{
					e->bit_field_value = e_from->bit_field_value;
					return 1;
				}
				case UINT :
				{
					e->bit_field_value = e_from->uint_value;
					return 1;
				}
				case INT :
				{
					e->bit_field_value = e_from->int_value;
					return 1;
				}
				case FLOAT :
				{
					if(dti_from->size == sizeof(float))
					{
						e->bit_field_value = e_from->float_value;
						return 1;
					}
					else if(dti_from->size == sizeof(double))
					{
						e->bit_field_value = e_from->double_value;
						return 1;
					}
					else
						return 0;
				}
				case LARGE_UINT :
				{
					e->bit_field_value = e_from->large_uint_value.limbs[0];
					return 1;
				}
				case LARGE_INT :
				{
					e->bit_field_value = e_from->large_int_value.raw_uint_value.limbs[0];
					return 1;
				}
				default :
					return 0;
			}
		}
		case UINT :
		{
			switch(dti_from->type)
			{
				case BIT_FIELD :
				{
					e->uint_value = e_from->bit_field_value;
					return 1;
				}
				case UINT :
				{
					e->uint_value = e_from->uint_value;
					return 1;
				}
				case INT :
				{
					e->uint_value = e_from->int_value;
					return 1;
				}
				case FLOAT :
				{
					if(dti_from->size == sizeof(float))
					{
						e->uint_value = e_from->float_value;
						return 1;
					}
					else if(dti_from->size == sizeof(double))
					{
						e->uint_value = e_from->double_value;
						return 1;
					}
					else
						return 0;
				}
				case LARGE_UINT :
				{
					e->uint_value = e_from->large_uint_value.limbs[0];
					return 1;
				}
				case LARGE_INT :
				{
					e->uint_value = e_from->large_int_value.raw_uint_value.limbs[0];
					return 1;
				}
				default :
					return 0;
			}
		}
		case INT :
		{
			switch(dti_from->type)
			{
				case BIT_FIELD :
				{
					e->int_value = e_from->bit_field_value;
					return 1;
				}
				case UINT :
				{
					e->int_value = e_from->uint_value;
					return 1;
				}
				case INT :
				{
					e->int_value = e_from->int_value;
					return 1;
				}
				case FLOAT :
				{
					if(dti_from->size == sizeof(float))
					{
						e->int_value = e_from->float_value;
						return 1;
					}
					else if(dti_from->size == sizeof(double))
					{
						e->int_value = e_from->double_value;
						return 1;
					}
					else
						return 0;
				}
				case LARGE_UINT :
				{
					e->int_value = e_from->large_uint_value.limbs[0];
					return 1;
				}
				case LARGE_INT :
				{
					e->int_value = e_from->large_int_value.raw_uint_value.limbs[0];
					return 1;
				}
				default :
					return 0;
			}
		}
		case FLOAT :
		{
			if(dti->size == sizeof(float))
			{
				switch(dti_from->type)
				{
					case BIT_FIELD :
					{
						e->float_value = e_from->bit_field_value;
						return 1;
					}
					case UINT :
					{
						e->float_value = e_from->uint_value;
						return 1;
					}
					case INT :
					{
						e->float_value = e_from->int_value;
						return 1;
					}
					case FLOAT :
					{
						if(dti_from->size == sizeof(float))
						{
							e->float_value = e_from->float_value;
							return 1;
						}
						else if(dti_from->size == sizeof(double))
						{
							e->float_value = e_from->double_value;
							return 1;
						}
						else
							return 0;
					}
					default :
						return 0;
				}
			}
			else if(dti->size == sizeof(double))
			{
				switch(dti_from->type)
				{
					case BIT_FIELD :
					{
						e->double_value = e_from->bit_field_value;
						return 1;
					}
					case UINT :
					{
						e->double_value = e_from->uint_value;
						return 1;
					}
					case INT :
					{
						e->double_value = e_from->int_value;
						return 1;
					}
					case FLOAT :
					{
						if(dti_from->size == sizeof(float))
						{
							e->double_value = e_from->float_value;
							return 1;
						}
						else if(dti_from->size == sizeof(double))
						{
							e->double_value = e_from->double_value;
							return 1;
						}
						else
							return 0;
					}
					default :
						return 0;
				}
			}
			else
				return 0;
		}
		case LARGE_UINT :
		{
			switch(dti_from->type)
			{
				case BIT_FIELD :
				{
					e->large_uint_value = get_uint256(e_from->bit_field_value);
					return 1;
				}
				case UINT :
				{
					e->large_uint_value = get_uint256(e_from->uint_value);
					return 1;
				}
				case INT :
				{
					e->large_uint_value = get_int256(e_from->int_value).raw_uint_value;
					return 1;
				}
				case LARGE_UINT :
				{
					e->large_uint_value = e_from->large_uint_value;
					return 1;
				}
				case LARGE_INT :
				{
					e->large_uint_value = e_from->large_int_value.raw_uint_value;
					return 1;
				}
				default :
					return 0;
			}
		}
		case LARGE_INT :
		{
			switch(dti_from->type)
			{
				case BIT_FIELD :
				{
					e->large_int_value = (int256){get_uint256(e_from->bit_field_value)};
					return 1;
				}
				case UINT :
				{
					e->large_int_value = (int256){get_uint256(e_from->uint_value)};
					return 1;
				}
				case INT :
				{
					e->large_int_value = get_int256(e_from->int_value);
					return 1;
				}
				case LARGE_UINT :
				{
					e->large_int_value = (int256){e_from->large_uint_value};
					return 1;
				}
				case LARGE_INT :
				{
					e->large_int_value = e_from->large_int_value;
					return 1;
				}
				default :
					return 0;
			}
		}
		default :
			return 0;
	}
}

user_value get_MIN_value_for_primitive_numeral_type_info(const data_type_info* dti)
{
	// NULL is always the least value you can imagine
	if(is_nullable_type_info(dti))
		return (*NULL_USER_VALUE);

	switch(dti->type)
	{
		case UINT :
			return (user_value){.uint_value = get_UINT64_MIN(dti->size)};
		case INT :
			return (user_value){.int_value = get_INT64_MIN(dti->size)};
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				return (user_value){.float_value = get_FLOAT_MIN()};
			else if(dti->size == sizeof(double))
				return (user_value){.double_value = get_DOUBLE_MIN()};
			else
				return (*NULL_USER_VALUE);
		}
		case LARGE_UINT :
			return (user_value){.large_uint_value = get_min_uint256()};
		case LARGE_INT :
			return (user_value){.large_int_value = bitwise_not_int256(get_bitmask_lower_n_bits_set_int256((dti->size * CHAR_BIT) - 1))};
		case BIT_FIELD :
			return (user_value){.bit_field_value = 0};
		default :
			return (*NULL_USER_VALUE);
	}
}

user_value get_MAX_value_for_primitive_numeral_type_info(const data_type_info* dti)
{
	switch(dti->type)
	{
		case UINT :
			return (user_value){.uint_value = get_UINT64_MAX(dti->size)};
		case INT :
			return (user_value){.int_value = get_INT64_MAX(dti->size)};
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				return (user_value){.float_value = get_FLOAT_MAX()};
			else if(dti->size == sizeof(double))
				return (user_value){.double_value = get_DOUBLE_MAX()};
			else
				return (*NULL_USER_VALUE);
		}
		case LARGE_UINT : // returns a uint256, with least significant (dti->size * CHAR_BIT) bits set to 1
			return (user_value){.large_uint_value = get_bitmask_lower_n_bits_set_uint256(dti->size * CHAR_BIT)};
		case LARGE_INT :
			return (user_value){.large_int_value = get_bitmask_lower_n_bits_set_int256((dti->size * CHAR_BIT) - 1)};
		case BIT_FIELD :
			return (user_value){.bit_field_value = ((dti->bit_field_size == 64) ? UINT64_MAX : ((UINT64_C(1) << dti->bit_field_size) - 1))};
		default :
			return (*NULL_USER_VALUE);
	}
}