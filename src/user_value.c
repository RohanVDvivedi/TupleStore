#include<user_value.h>

#include<tuple_def.h>

#include<primitive_numeral_types.h>

#include<inttypes.h>
#include<stdlib.h>

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

user_value get_MIN_user_value(const element_def* ele_d)
{
	if(is_NULLable_element_def(ele_d))
		return (*NULL_USER_VALUE);

	if(is_numeral_type_element_def(ele_d))
		return get_MIN_value_for_numeral_element_def(ele_d);
	else
		return get_MIN_value_for_string_OR_blob_element_def(ele_d);
}

user_value get_MIN_user_value_for_primitive_numeral_type(const data_type_info* dti)
{
	if(is_primitive_numeral_type_info(dti))
		return get_MIN_value_for_primitive_numeral_type_info(dti);
	else
		return (*NULL_USER_VALUE);
}

user_value get_MAX_user_value_for_primitive_numeral_type(const data_type_info* dti)
{
	if(is_primitive_numeral_type_info(dti))
		return get_MAX_value_for_primitive_numeral_type_info(dti);
	else
		return (*NULL_USER_VALUE);
}