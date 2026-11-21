#ifndef FAST_PATH_COMPARATORS_H
#define FAST_PATH_COMPARATORS_H

#include<tuplestore/datum.h>

// bit_field_value and uint_value
int fast_path_compare_uint64(const datum* uval1, const datum* uval2);

// int_value
int fast_path_compare_int64(const datum* uval1, const datum* uval2);

// float
int fast_path_compare_float(const datum* uval1, const datum* uval2);

// double
int fast_path_compare_double(const datum* uval1, const datum* uval2);

// large_uint_value
int fast_path_compare_uint256(const datum* uval1, const datum* uval2);

// large_int_value
int fast_path_compare_int256(const datum* uval1, const datum* uval2);

// string_or_binary_value
int fast_path_compare_string_binary(const datum* uval1, const datum* uval2);

#endif