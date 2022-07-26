#include<user_value.h>
#include<stdlib.h>

const user_value NULL_USER_VALUE = {.is_NULL = 1};

int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}