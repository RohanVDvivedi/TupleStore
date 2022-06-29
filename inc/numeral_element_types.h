#ifndef NUMERAL_ELEMENT_TYPES_H
#define NUMERAL_ELEMENT_TYPES_H

#include<tuple_def.h>

#include<user_value.h>

// all numeral types are fixed sizes element defs
// which represent numbers

int is_numeral_type_element_def(const element_def* ele_d);

int compare_numeral_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

void set_numeral_element(void* e, const element_def* ele_d, const user_value* uval);

void set_numeral_element_from_element(void* e, const element_def* ele_d, const void* e_from, const element_def* ele_d_from);

user_value get_value_from_numeral_element(const void* e, const element_def* ele_d);

#endif