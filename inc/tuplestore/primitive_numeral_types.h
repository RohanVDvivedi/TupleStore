#ifndef PRIMITIVE_NUMERAL_TYPES_H
#define PRIMITIVE_NUMERAL_TYPES_H

#include<tuplestore/data_type_info.h>
#include<tuplestore/datum.h>

int is_primitive_numeral_type_info(const data_type_info* dti);

// before calling any of the below functions, you must ensure that the called dti-s are primitive_numeral_type_infos

int compare_primitive_numeral_type(const datum* e1, const data_type_info* dti_1, const datum* e2, const data_type_info* dti_2);

int compare_primitive_numeral_type2(const datum* e1, const datum* e2, const data_type_info* dti);

int type_cast_primitive_numeral_type(datum* e, const data_type_info* dti, const datum* e_from, const data_type_info* dti_from);

datum get_MIN_value_for_primitive_numeral_type_info(const data_type_info* dti);

datum get_MAX_value_for_primitive_numeral_type_info(const data_type_info* dti);

#endif