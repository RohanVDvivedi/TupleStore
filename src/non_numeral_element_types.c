#include<non_numeral_element_types.h>

int is_string_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == STRING) || (ele_d->type == VAR_STRING);
}

int is_blob_type_element_def(const element_def* ele_d)
{
	return (ele_d->type == BLOB) || (ele_d->type == VAR_BLOB);
}