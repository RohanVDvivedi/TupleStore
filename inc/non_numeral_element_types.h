#ifndef NON_NUMERAL_ELEMENT_TYPES_H
#define NON_NUMERAL_ELEMENT_TYPES_H

#include<tuple_def.h>
#include<user_value.h>

int is_string_type_element_def(const element_def* ele_d);

int is_blob_type_element_def(const element_def* ele_d);

int is_variable_sized_non_numeral_element_def(const element_def* ele_d);

// for variable sized blob or string i.e. VAR_BLOB or VAR_STRING,
// the first element_def.size_specifier_prefix_size number of bytes are used to store the size of the actual data
// and then the actual data begins
// VAR_STRING are not NULL terminated, unlike the STRING which may be NULL terminated, if the string length is lesser than its fixed size

// this only includes the actual size of the data
// does not include the bytes required to store the size of this data (i.e. does not include element_def.size_specifier_prefix_size)
uint32_t get_data_size_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d);

// returns pointer in e, such that the returned pointer directly points to data of this
// non-numeral variable sized element
const void* get_data_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d);

// this includes the complete size of the data
uint32_t get_element_size_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d);

uint32_t get_string_length_for_string_type_element(const void* e, const element_def* ele_d);

int compare_string_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

int compare_blob_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

void set_variable_sized_non_numeral_element(void* e, const element_def* ele_d, const user_value* uval);

void set_string_OR_blob_element(void* e, const element_def* ele_d, const user_value* uval);

// returns 0 if the assignement fails
// it may fails because the types are in-compatible
// both of them must pass is_string_type_element_def OR is_string_type_blob_def
int set_string_OR_blob_element_from_element(void* e, const element_def* ele_d, const void* e_from, const element_def* ele_d_from);

// returns user value that will point to blob or string of this element
// it does not allocate memory
user_value get_value_from_non_numeral_element(const void* e, const element_def* ele_d);

#endif