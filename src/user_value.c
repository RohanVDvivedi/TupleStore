#include<tuplestore/user_value.h>

#include<tuplestore/primitive_numeral_types.h>

#include<cutlery/cutlery_stds.h>

user_value const * const NULL_USER_VALUE = &((const user_value){.is_NULL = 1,});

user_value const * const ZERO_USER_VALUE = &((const user_value){.is_NULL = 0,});
user_value const * const EMPTY_USER_VALUE = ZERO_USER_VALUE;

uint32_t get_element_count_for_user_value(const user_value* uval, const data_type_info* dti)
{
	if(!is_container_type_info(dti))
		return 0;

	if(is_user_value_NULL(uval))
		return 0;

	if(dti->type == STRING)
		return uval->string_size;
	else if(dti->type == BLOB)
		return uval->blob_size;
	else if(dti->type == TUPLE)
		return get_element_count_for_container_type_info(dti, uval->tuple_value);
	else if(dti->type == ARRAY)
		return get_element_count_for_container_type_info(dti, uval->array_value);

	// never reaches here
	return 0;
}

int get_containee_for_user_value(user_value* uval_c, const user_value* uval, const data_type_info* dti, uint32_t index)
{
	if(!is_container_type_info(dti))
		return 0;

	if(is_user_value_NULL(uval))
	{
		uval_c->is_NULL = 1;
		return 1;
	}

	if(index >= get_element_count_for_user_value(uval, dti))
		return 0;

	if(dti->type == STRING || dti->type == BLOB)
	{
		uval_c->is_NULL = 0;
		uval_c->uint_value = (((const unsigned char *)(uval->string_or_blob_value))[index] & UINT64_C(0xff));
		return 1;
	}
	else if(dti->type == TUPLE)
	{
		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
		return get_user_value_to_containee_from_container(uval_c, dti, uval->tuple_value, index, &containee_pos_info);
	}
	else if(dti->type == ARRAY)
	{
		data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
		return get_user_value_to_containee_from_container(uval_c, dti, uval->array_value, index, &containee_pos_info);
	}

	// never reaches here
	return 0;
}

int can_compare_user_value(const data_type_info* dti1, const data_type_info* dti2)
{
	if(are_identical_type_info(dti1, dti2)) // logically same exact types, this is essential for TUPLE and ARRAY types
		return 1;
	else if(is_primitive_numeral_type_info(dti1) && is_primitive_numeral_type_info(dti2) && can_compare_primitive_numeral_type_infos(dti1, dti2)) // both are primitive numeral types
		return 1;
	else if((dti1->type == STRING || dti1->type == BLOB || dti1->type == ARRAY) && (dti2->type == STRING || dti2->type == BLOB || dti2->type == ARRAY)) // STRING, BLOB and ARRAY are internally comparable, if their containee types are comparable
		return can_compare_user_value(dti1->containee, dti2->containee);
	else
		return 0;
}

// before calling this function the dti1 and dti2 must pass this check : can_compare_user_value(dti1, dti2)
static int compare_user_value_internal(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2)
{
	if(is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 0;
	else if(is_user_value_NULL(uval1) && !is_user_value_NULL(uval2))
		return -1;
	else if(!is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 1;

	if(is_primitive_numeral_type_info(dti1)) // both are primitive types and are comparable
		return compare_primitive_numeral_type(uval1, dti1, uval2, dti2);
	else if(dti1->type == TUPLE) // both are the same tuple types
	{
		int cmp = 0;
		uint32_t element_count = get_element_count_for_user_value(uval1, dti1);
		for(uint32_t i = 0; i < element_count && cmp == 0; i++)
		{
			const data_type_info* child_dti1 = get_data_type_info_for_containee_of_container_without_data(dti1, i);
			user_value child_value1;
			if(!get_containee_for_user_value(&child_value1, uval1, dti1, i))
				return -2;

			const data_type_info* child_dti2 = get_data_type_info_for_containee_of_container_without_data(dti2, i);
			user_value child_value2;
			if(!get_containee_for_user_value(&child_value2, uval2, dti2, i))
				return -2;

			cmp = compare_user_value_internal(&child_value1, child_dti1, &child_value2, child_dti2);
		}
		return cmp;
	}
	else // they both are a 9-combination of STRING, BLOB and ARRAY of comparable types
	{
		int cmp = 0;
		uint32_t element_count1 = get_element_count_for_user_value(uval1, dti1);
		uint32_t element_count2 = get_element_count_for_user_value(uval2, dti2);
		uint32_t element_count = min(element_count1, element_count2);
		for(uint32_t i = 0; i < element_count && cmp == 0; i++) // perform comparison over the minimum element count of both the containers
		{
			const data_type_info* child_dti1 = get_data_type_info_for_containee_of_container_without_data(dti1, i);
			user_value child_value1;
			if(!get_containee_for_user_value(&child_value1, uval1, dti1, i))
				return -2;

			const data_type_info* child_dti2 = get_data_type_info_for_containee_of_container_without_data(dti2, i);
			user_value child_value2;
			if(!get_containee_for_user_value(&child_value2, uval2, dti2, i))
				return -2;

			cmp = compare_user_value_internal(&child_value1, child_dti1, &child_value2, child_dti2);
		}
		if(cmp == 0 && (element_count1 != element_count2))
		{
			if(element_count1 > element_count2)
				cmp = 1;
			else
				cmp = -1;
		}
		return cmp;
	}
}

int compare_user_value(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2)
{
	// first check that the element types attempted to be compared are comparable types
	// it can be recursive fro nested arrays 
	if(!can_compare_user_value(dti1, dti2))
		return -2;

	return compare_user_value_internal(uval1, dti1, uval2, dti2);
}

static int compare_user_value_internal2(const user_value* uval1, const user_value* uval2, const data_type_info* dti)
{
	if(is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 0;
	else if(is_user_value_NULL(uval1) && !is_user_value_NULL(uval2))
		return -1;
	else if(!is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 1;

	if(is_primitive_numeral_type_info(dti)) // both are primitive types and are comparable
		return compare_primitive_numeral_type2(uval1, uval2, dti);
	else if(dti->type == TUPLE) // both are the same tuple types
	{
		int cmp = 0;
		uint32_t element_count = get_element_count_for_user_value(uval1, dti);
		for(uint32_t i = 0; i < element_count && cmp == 0; i++)
		{
			const data_type_info* child_dti = get_data_type_info_for_containee_of_container_without_data(dti, i);

			user_value child_value1;
			if(!get_containee_for_user_value(&child_value1, uval1, dti, i))
				return -2;

			user_value child_value2;
			if(!get_containee_for_user_value(&child_value2, uval2, dti, i))
				return -2;

			cmp = compare_user_value_internal2(&child_value1, &child_value2, child_dti);
		}
		return cmp;
	}
	else if(dti->type == STRING || dti->type == BLOB)
	{
		int cmp = 0;
		uint32_t element_count1 = get_element_count_for_user_value(uval1, dti);
		uint32_t element_count2 = get_element_count_for_user_value(uval2, dti);
		uint32_t element_count = min(element_count1, element_count2);

		for(uint32_t i = 0; i < element_count && cmp == 0; i++)
			cmp = compare_numbers( (*((const unsigned char*)(uval1->string_or_blob_value + i))), (*((const unsigned char*)(uval2->string_or_blob_value + i))) );

		if(cmp == 0 && (element_count1 != element_count2))
		{
			if(element_count1 > element_count2)
				cmp = 1;
			else
				cmp = -1;
		}

		return cmp;
	}
	else // they both are a 9-combination of STRING, BLOB and ARRAY of comparable types
	{
		int cmp = 0;
		uint32_t element_count1 = get_element_count_for_user_value(uval1, dti);
		uint32_t element_count2 = get_element_count_for_user_value(uval2, dti);
		uint32_t element_count = min(element_count1, element_count2);
		for(uint32_t i = 0; i < element_count && cmp == 0; i++) // perform comparison over the minimum element count of both the containers
		{
			const data_type_info* child_dti = get_data_type_info_for_containee_of_container_without_data(dti, i);

			user_value child_value1;
			if(!get_containee_for_user_value(&child_value1, uval1, dti, i))
				return -2;

			user_value child_value2;
			if(!get_containee_for_user_value(&child_value2, uval2, dti, i))
				return -2;

			cmp = compare_user_value_internal2(&child_value1, &child_value2, child_dti);
		}
		if(cmp == 0 && (element_count1 != element_count2))
		{
			if(element_count1 > element_count2)
				cmp = 1;
			else
				cmp = -1;
		}
		return cmp;
	}
}

int compare_user_value2(const user_value* uval1, const user_value* uval2, const data_type_info* dti)
{
	return compare_user_value_internal2(uval1, uval2, dti);
}

uint64_t hash_user_value(const user_value* uval, const data_type_info* dti, tuple_hasher* th)
{
	if(is_user_value_NULL(uval)) // no bytes to hash
		return th->hash;

	if(dti->type == BIT_FIELD)
	{
		for(uint32_t i = 0; i < dti->bit_field_size; i++)
		{
			char bit_data = !!((uval->bit_field_value >> i) & UINT64_C(1));
			tuple_hash_byte(th, bit_data);
		}
		return th->hash;
	}
	else if(!is_container_type_info(dti)) // case for UINT, INT, FLOAT and LARGE_UINT
	{
		// has to be fixed size
		char serialized_value[sizeof(user_value)];

		set_user_value_for_type_info(dti, serialized_value, 0, 0 /* has to be fixed sized, hence this parameter is never used*/, uval);

		return tuple_hash_bytes(th, (const uint8_t*)serialized_value, get_size_for_type_info(dti ,serialized_value));
	}
	else if(dti->type == STRING || dti->type == BLOB)
	{
		return tuple_hash_bytes(th, uval->string_or_blob_value, uval->string_or_blob_size);
	}
	else
	{
		for(uint32_t i = 0; i < get_element_count_for_user_value(uval, dti); i++)
		{
			const data_type_info* child_dti = get_data_type_info_for_containee_of_container_without_data(dti, i);
			user_value child_value;
			if(!get_containee_for_user_value(&child_value, uval, dti, i))
				continue;
			hash_user_value(&child_value, child_dti, th);
		}
		return th->hash;
	}
}

#include<tuplestore/page_layout_util.h>

void print_user_value(const user_value* uval, const data_type_info* dti)
{
	if(is_user_value_NULL(uval))
	{
		printf("NULL");
		return;
	}
	switch(dti->type)
	{
		case BIT_FIELD :
		{
			printf("%"PRIx64, uval->bit_field_value);
			break;
		}
		case UINT :
		{
			printf("%"PRIu64, uval->uint_value);
			break;
		}
		case INT :
		{
			printf("%"PRId64, uval->int_value);
			break;
		}
		case FLOAT :
		{
			if(dti->size == sizeof(float))
				printf("%f", uval->float_value);
			else if(dti->size == sizeof(double))
				printf("%lf", uval->double_value);
			else if(dti->size == sizeof(long double))
				printf("%Lf", uval->long_double_value);
			break;
		}
		case LARGE_UINT :
		{
			print_uint256(uval->large_uint_value);
			break;
		}
		case STRING :
		{
			printf("\"%.*s\"", uval->string_size, ((const char*)(uval->string_value)));
			break;
		}
		case BLOB :
		{
			printf("BLOB<%"PRIu32">[", uval->blob_size);
			for(uint32_t i = 0; i < uval->blob_size; i++)
				printf(" 0x%2"PRIx8, ((const uint8_t*)(uval->blob_value))[i]);
			printf("]");
			break;
		}
		case TUPLE :
		{
			printf("(%s)<%"PRIu32">(", dti->type_name, get_size_for_type_info(dti, uval->tuple_value));
			for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, uval->tuple_value); i++)
			{
				if(i != 0)
					printf(", ");
				const data_position_info* containee_pos_info = dti->containees + i;
				user_value child_uval;
				if(!get_user_value_to_containee_from_container_CONTAINITY_UNSAFE(&child_uval, dti, uval->tuple_value, i, (data_positional_info*)(&(containee_pos_info->al))))
					continue;
				if(is_variable_sized_type_info(containee_pos_info->al.type_info) && !is_user_value_NULL(&child_uval))
					printf("[%"PRIu32"]->", read_value_from_page(uval->tuple_value + containee_pos_info->al.byte_offset_to_byte_offset, dti->max_size));
				print_user_value(&child_uval, containee_pos_info->al.type_info);
			}
			printf(")");
			break;
		}
		case ARRAY :
		{
			printf("ARRAY<%"PRIu32">[", get_size_for_type_info(dti, uval->array_value));
			for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, uval->array_value); i++)
			{
				if(i != 0)
					printf(", ");
				data_positional_info containee_pos_info = INVALID_DATA_POSITIONAL_INFO;
				get_data_positional_info_for_containee_of_container_CONTAINITY_UNSAFE(dti, uval->array_value, i, &containee_pos_info);
				user_value child_uval;
				if(!get_user_value_to_containee_from_container_CONTAINITY_UNSAFE(&child_uval, dti, uval->array_value, i, &containee_pos_info))
					continue;
				if(is_variable_sized_type_info(containee_pos_info.type_info) && !is_user_value_NULL(&child_uval))
					printf("[%"PRIu32"]->", read_value_from_page(uval->tuple_value + containee_pos_info.byte_offset_to_byte_offset, dti->max_size));
				print_user_value(&child_uval, containee_pos_info.type_info);
			}
			printf("]");
			break;
		}
	}
}