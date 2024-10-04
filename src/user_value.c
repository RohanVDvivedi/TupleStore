#include<user_value.h>

#include<primitive_numeral_types.h>

#include<cutlery_stds.h>

user_value const * const DEFAULT_USER_VALUE = &((const user_value){});

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
	else if(dti->type == STRING)
	{
		uint32_t hash = 0;
		for(uint32_t i = 0; i < uval->string_size; i++)
			hash ^= hash_func(uval->string_value + i, 1);
		return hash;
	}
	else if(dti->type == BLOB)
	{
		uint32_t hash = 0;
		for(uint32_t i = 0; i < uval->blob_size; i++)
			hash ^= hash_func(uval->blob_value + i, 1);
		return hash;
	}
	else if(dti->type == TUPLE)
		return hash_data_for_type_info(dti, uval->tuple_value, hash_func);
	else if(dti->type == ARRAY)
		return hash_data_for_type_info(dti, uval->array_value, hash_func);
	else
		return 0;
}