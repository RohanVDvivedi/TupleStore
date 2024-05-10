#include<user_value.h>

#include<tuple_def.h>

#include<numeral_element_types.h>
#include<string_or_blob_element_types_util.h>

#include<inttypes.h>
#include<stdlib.h>

#include<cutlery_stds.h>

user_value const * const DEFAULT_USER_VALUE = &((const user_value){});

user_value const * const NULL_USER_VALUE = &((const user_value){.is_NULL = 1});
user_value const * const ZERO_USER_VALUE = &((const user_value){.is_NULL = 0});
user_value const * const EMPTY_USER_VALUE = &((const user_value){.is_NULL = 0, .data = NULL, .data_size = 0});

int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}

user_value clone_user_value_with_data(const user_value* uval)
{
	void* new_data = malloc(uval->data_size);
	// generally this call is to be executed, while inserting default value of the element defs,
	// so it will be called at the beginning of the application
	// this allocation must be insignificant in quantity (too small)
	// and is expected to not fail, if it fails you are on your own
	// no NULL handling done here, too bad, just let the OS kill you

	memory_move(new_data, uval->data, uval->data_size);
	user_value res = {.data = new_data, .data_size = uval->data_size};
	return res;
}

user_value get_MIN_user_value(const element_def* ele_d)
{
	if(is_NULLable_element_def(ele_d))
		return (*NULL_USER_VALUE);

	if(is_numeral_type_element_def(ele_d))
		return get_MIN_value_for_numeral_element_def(ele_d);
	else
		return get_MIN_value_for_string_OR_blob_element_def(ele_d);
}

user_value get_MAX_user_value_for_numeral_element_def(const element_def* ele_d)
{
	if(is_numeral_type_element_def(ele_d))
		return get_MAX_value_for_numeral_element_def(ele_d);
	else
		return (*NULL_USER_VALUE);
}

user_value get_MAX_user_value_for_string_OR_blob_element_def(const element_def* ele_d, uint32_t max_length, int* memory_allocation_error)
{
	// limit the length of max_element that get's generated
	if(is_fixed_sized_element_def(ele_d))
		max_length = min(max_length, ele_d->size);
	else if(ele_d->size_specifier_prefix_size < 4) // 1U << (`4` * CHAR_BIT), overflows hence the check
		max_length = min(max_length, 1U << (ele_d->size_specifier_prefix_size * CHAR_BIT));

	// allocate new_data and set all it's max_length bytes to CHAR_MAX (max value of char data type)
	void* new_data = malloc(max_length);
	if(new_data == NULL)
	{
		(*memory_allocation_error) = 1;
		return (*NULL_USER_VALUE);
	}
	memory_set(new_data, SIGNED_MAX_VALUE_OF(char), max_length);

	user_value res = {.data = new_data, .data_size = max_length};
	return res;
}

void print_user_value(const user_value* uval, const element_def* ele_d)
{
	if(is_user_value_NULL(uval))
	{
		printf("NULL");
		return;
	}

	switch(ele_d->type)
	{
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
			if(ele_d->size == sizeof(float))
				printf("%f", uval->float_value);
			else if(ele_d->size == sizeof(double))
				printf("%lf", uval->double_value);
			else if(ele_d->size == sizeof(long double))
				printf("%Lf", uval->long_double_value);
			break;
		}
		case LARGE_UINT :
		{
			print_large_uint(uval->large_uint_value);
			break;
		}
		case BIT_FIELD :
		{
			printf("%"PRIx64, uval->bit_field_value);
			break;
		}
		case STRING :
		case VAR_STRING :
		{
			printf("\"%.*s\"", uval->data_size, ((const char*)(uval->data)));
			break;
		}
		case BLOB :
		case VAR_BLOB :
		{
			printf("BLOB(%"PRIu32")[", uval->data_size);
			for(uint32_t i = 0; i < uval->data_size; i++)
				printf(" 0x%2"PRIx8, ((const uint8_t*)(uval->data))[i]);
			printf("]");
			break;
		}
	}
}