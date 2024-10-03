#ifndef PRIMITIVE_NUMERAL_TYPES_H
#define PRIMITIVE_NUMERAL_TYPES_H

#include<data_type_info.h>
#include<user_value.h>

int is_primitive_numeral_type_info(const data_type_info* dti);

// before calling any of the below functions, you must ensure that the called dti-s are primitive_numeral_type_infos

int can_compare_primitive_numeral_type_infos(const data_type_info* dti_1, const data_type_info* dti_2);

int compare_primitive_numeral_type(const user_value* e1, const data_type_info* dti_1, const user_value* e2, const data_type_info* dti_2);

int can_type_cast_primitive_numeral_type(const data_type_info* dti, const data_type_info* dti_from);

int type_cast_primitive_numeral_type(user_value* e, const data_type_info* dti, const user_value* e_from, const data_type_info* dti_from);

user_value get_value_from_numeral_element(const user_value* e, const data_type_info* dti);

user_value get_MIN_value_for_primitive_numeral_type_info(const data_type_info* dti);

user_value get_MAX_value_for_primitive_numeral_type_info(const data_type_info* dti);

#endif