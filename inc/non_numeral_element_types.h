#ifndef NON_NUMERAL_ELEMENT_TYPES_H
#define NON_NUMERAL_ELEMENT_TYPES_H

#include<tuple_def.h>

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

// this includes the complete size of the data
uint32_t get_element_size_for_variable_sized_non_numeral_element(const void* e, const element_def* ele_d);

#endif