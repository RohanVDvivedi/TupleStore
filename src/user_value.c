#include<user_value.h>
#include<stdlib.h>

int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}