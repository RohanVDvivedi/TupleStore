#ifndef STRING_OR_BLOB_ELEMENT_TYPES_H
#define STRING_OR_BLOB_ELEMENT_TYPES_H

#include<tuple_def.h>
#include<user_value.h>

int is_string_type_element_def(const element_def* ele_d);

int is_blob_type_element_def(const element_def* ele_d);

int is_string_OR_blob_type_element_def(const element_def* ele_d);

int is_fixed_sized_string_OR_blob_element_def(const element_def* ele_d);

int is_variable_sized_string_OR_blob_element_def(const element_def* ele_d);

// for variable sized blob or string i.e. VAR_BLOB or VAR_STRING,
// the first element_def.size_specifier_prefix_size number of bytes are used to store the size of the actual data
// and then the actual data begins
// VAR_STRING are not NULL terminated, unlike the STRING which may be NULL terminated, if the string length is lesser than its fixed size

// this includes the complete size of the data
uint32_t get_element_size_for_string_OR_blob_element(const void* e, const element_def* ele_d);

uint32_t get_element_size_from_user_value_for_string_OR_blob_element(const user_value* uval, const element_def* ele_d);

uint32_t get_string_length_for_string_type_element(const void* e, const element_def* ele_d);

// compares as unsigned char-s, depends on strncmp
int compare_string_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

// compares as unsigned char-s, depends on memcmp
int compare_blob_type_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

uint64_t hash_string_OR_blob_type_element(const void* e, const element_def* ele_d, uint64_t (*hash_func)(const void* data, uint32_t size));

void set_string_OR_blob_element(void* e, const element_def* ele_d, const user_value* uval);

// returns 0 if the assignement fails
// it may fails because the types are in-compatible
// both of them must pass is_string_type_element_def OR is_string_type_blob_def
int set_string_OR_blob_element_from_element(void* e, const element_def* ele_d, const void* e_from, const element_def* ele_d_from);

// returns user value that will point to blob or string of this element
// it does not allocate memory
user_value get_value_from_string_OR_blob_element(const void* e, const element_def* ele_d);

// gives out a length of 0 bytes long STRING, BLOB, VAR_STRING or VAR_BLOB
user_value get_MIN_value_for_string_OR_blob_element_def(const element_def* ele_d);

#endif