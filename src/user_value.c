#include<user_value.h>

#include<primitive_numeral_types.h>

#include<cutlery_stds.h>

user_value const * const NULL_USER_VALUE = &((const user_value){.is_NULL = 1, .is_OUT_OF_BOUNDS = 0,});
user_value const * const OUT_OF_BOUNDS_USER_VALUE = &((const user_value){.is_NULL = 1, .is_OUT_OF_BOUNDS = 1,});

user_value const * const ZERO_USER_VALUE = &((const user_value){.is_NULL = 0, .is_OUT_OF_BOUNDS = 0,});
user_value const * const EMPTY_USER_VALUE = ZERO_USER_VALUE;

int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}

int is_user_value_OUT_OF_BOUNDS(const user_value* uval)
{
	return (uval != NULL) && uval->is_OUT_OF_BOUNDS;
}

uint32_t get_element_count_for_user_value(const user_value* uval, const data_type_info* dti)
{
	if(!is_container_type_info(dti))
		return 0;

	if(is_user_value_NULL(uval) || is_user_value_OUT_OF_BOUNDS(uval))
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

const user_value get_containee_for_user_value(const user_value* uval, const data_type_info* dti, uint32_t index)
{
	if(!is_container_type_info(dti))
		return (*OUT_OF_BOUNDS_USER_VALUE);

	if(is_user_value_NULL(uval) || is_user_value_OUT_OF_BOUNDS(uval))
		return (*OUT_OF_BOUNDS_USER_VALUE);

	if(index >= get_element_count_for_user_value(uval, dti))
		return (*OUT_OF_BOUNDS_USER_VALUE);

	if(dti->type == STRING)
		return (user_value){.uint_value = (((const unsigned char *)(uval->string_value))[index] & UINT64_C(0xff))};
	else if(dti->type == BLOB)
		return (user_value){.uint_value = (((const unsigned char *)(uval->blob_value))[index] & UINT64_C(0xff))};
	else if(dti->type == TUPLE)
		return get_user_value_to_containee_from_container(dti, uval->tuple_value, index);
	else if(dti->type == ARRAY)
		return get_user_value_to_containee_from_container(dti, uval->array_value, index);

	// never reaches here
	return (*OUT_OF_BOUNDS_USER_VALUE);
}

int can_compare_user_value(const data_type_info* dti1, const data_type_info* dti2)
{
	// TODO
}

int compare_user_value(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2)
{
	if(is_user_value_OUT_OF_BOUNDS(uval1) || is_user_value_OUT_OF_BOUNDS(uval2))
		return -2;

	if(!can_compare_user_value(dti1, dti2))
		return -2;

	if(is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 0;
	else if(is_user_value_NULL(uval1) && !is_user_value_NULL(uval2))
		return -1;
	else if(!is_user_value_NULL(uval1) && is_user_value_NULL(uval2))
		return 1;

	// TODO
}

uint64_t hash_user_value(const user_value* uval, const data_type_info* dti, uint64_t (*hash_func)(const void* data, uint32_t size))
{
	if(is_user_value_OUT_OF_BOUNDS(uval))
		return 0;

	if(is_user_value_NULL(uval))
		return 0;

	if(dti->type == BIT_FIELD)
	{
		uint32_t hash = 0;
		for(uint32_t i = 0; i < dti->bit_field_size; i++)
		{
			char bit_data = !!((uval->bit_field_value >> i) & UINT64_C(1));
			hash ^= hash_func(&bit_data, 1);
		}
		return hash;
	}
	else if(!is_container_type_info(dti)) // case for UINT, INT, FLOAT and LARGE_UINT
	{
		// has to be fixed size
		char serialized_value[sizeof(user_value)];

		set_user_value_for_type_info(dti, serialized_value, 0, 0 /* has to be fixed sized, hence this parameter is never used*/, uval);

		return hash_func(serialized_value, get_size_for_type_info(dti ,serialized_value));
	}
	else
	{
		uint32_t hash = 0;
		for(uint32_t i = 0; i < get_element_count_for_user_value(uval, dti); i++)
		{
			const data_type_info* child_dti = get_data_type_info_for_containee_of_container_without_data(dti, i);
			if(child_dti == NULL)
				return -2;
			const user_value child_value = get_containee_for_user_value(uval, dti, i);
			hash ^= hash_user_value(&child_value, child_dti, hash_func);
		}
		return hash;
	}
}

#include<page_layout_util.h>

void print_user_value(const user_value* uval, const data_type_info* dti)
{
	if(is_user_value_NULL(uval))
	{
		printf("NULL");
		return;
	}
	if(is_user_value_OUT_OF_BOUNDS(uval))
	{
		printf("OUT_OF_BOUNDS");
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
				data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, uval->tuple_value, i);
				if(is_variable_sized_type_info(containee_pos_info.type_info))
					printf("{%"PRIu32"}->", read_value_from_page(uval->tuple_value + containee_pos_info.byte_offset_to_byte_offset, dti->max_size));
				const user_value child_uval = get_user_value_to_containee_from_container(dti, uval->tuple_value, i);
				print_user_value(&child_uval, containee_pos_info.type_info);
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
				data_position_info containee_pos_info = get_data_position_info_for_containee_of_container(dti, uval->array_value, i);
				if(is_variable_sized_type_info(containee_pos_info.type_info))
					printf("{%"PRIu32"}->", read_value_from_page(uval->array_value + containee_pos_info.byte_offset_to_byte_offset, dti->max_size));
				const user_value child_uval = get_user_value_to_containee_from_container(dti, uval->array_value, i);
				print_user_value(&child_uval, containee_pos_info.type_info);
			}
			printf("]");
			break;
		}
	}
}