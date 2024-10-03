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