#ifndef DATA_TYPE_INFO_DEFAULTS_H
#define DATA_TYPE_INFO_DEFAULTS_H

#define declare_bit_field_nullable_type(_size) \
extern data_type_info* BIT_FIELD_ ## _size ## _NULLABLE;

declare_bit_field_nullable_type(1)
declare_bit_field_nullable_type(2)
declare_bit_field_nullable_type(3)
declare_bit_field_nullable_type(4)
declare_bit_field_nullable_type(5)
declare_bit_field_nullable_type(6)
declare_bit_field_nullable_type(7)
declare_bit_field_nullable_type(8)
declare_bit_field_nullable_type(9)
declare_bit_field_nullable_type(10)
declare_bit_field_nullable_type(11)
declare_bit_field_nullable_type(12)
declare_bit_field_nullable_type(13)
declare_bit_field_nullable_type(14)
declare_bit_field_nullable_type(15)
declare_bit_field_nullable_type(16)
declare_bit_field_nullable_type(17)
declare_bit_field_nullable_type(18)
declare_bit_field_nullable_type(19)
declare_bit_field_nullable_type(20)
declare_bit_field_nullable_type(21)
declare_bit_field_nullable_type(22)
declare_bit_field_nullable_type(23)
declare_bit_field_nullable_type(24)
declare_bit_field_nullable_type(25)
declare_bit_field_nullable_type(26)
declare_bit_field_nullable_type(27)
declare_bit_field_nullable_type(28)
declare_bit_field_nullable_type(29)
declare_bit_field_nullable_type(30)
declare_bit_field_nullable_type(31)
declare_bit_field_nullable_type(32)
declare_bit_field_nullable_type(33)
declare_bit_field_nullable_type(34)
declare_bit_field_nullable_type(35)
declare_bit_field_nullable_type(36)
declare_bit_field_nullable_type(37)
declare_bit_field_nullable_type(38)
declare_bit_field_nullable_type(39)
declare_bit_field_nullable_type(40)
declare_bit_field_nullable_type(41)
declare_bit_field_nullable_type(42)
declare_bit_field_nullable_type(43)
declare_bit_field_nullable_type(44)
declare_bit_field_nullable_type(45)
declare_bit_field_nullable_type(46)
declare_bit_field_nullable_type(47)
declare_bit_field_nullable_type(48)
declare_bit_field_nullable_type(49)
declare_bit_field_nullable_type(50)
declare_bit_field_nullable_type(51)
declare_bit_field_nullable_type(52)
declare_bit_field_nullable_type(53)
declare_bit_field_nullable_type(54)
declare_bit_field_nullable_type(55)
declare_bit_field_nullable_type(56)
declare_bit_field_nullable_type(57)
declare_bit_field_nullable_type(58)
declare_bit_field_nullable_type(59)
declare_bit_field_nullable_type(60)
declare_bit_field_nullable_type(61)
declare_bit_field_nullable_type(62)
declare_bit_field_nullable_type(63)
declare_bit_field_nullable_type(64)

#define declare_bit_field_non_nullable_type(_size) \
extern data_type_info* BIT_FIELD_ ## _size ## _NON_NULLABLE;

declare_bit_field_non_nullable_type(1)
declare_bit_field_non_nullable_type(2)
declare_bit_field_non_nullable_type(3)
declare_bit_field_non_nullable_type(4)
declare_bit_field_non_nullable_type(5)
declare_bit_field_non_nullable_type(6)
declare_bit_field_non_nullable_type(7)
declare_bit_field_non_nullable_type(8)
declare_bit_field_non_nullable_type(9)
declare_bit_field_non_nullable_type(10)
declare_bit_field_non_nullable_type(11)
declare_bit_field_non_nullable_type(12)
declare_bit_field_non_nullable_type(13)
declare_bit_field_non_nullable_type(14)
declare_bit_field_non_nullable_type(15)
declare_bit_field_non_nullable_type(16)
declare_bit_field_non_nullable_type(17)
declare_bit_field_non_nullable_type(18)
declare_bit_field_non_nullable_type(19)
declare_bit_field_non_nullable_type(20)
declare_bit_field_non_nullable_type(21)
declare_bit_field_non_nullable_type(22)
declare_bit_field_non_nullable_type(23)
declare_bit_field_non_nullable_type(24)
declare_bit_field_non_nullable_type(25)
declare_bit_field_non_nullable_type(26)
declare_bit_field_non_nullable_type(27)
declare_bit_field_non_nullable_type(28)
declare_bit_field_non_nullable_type(29)
declare_bit_field_non_nullable_type(30)
declare_bit_field_non_nullable_type(31)
declare_bit_field_non_nullable_type(32)
declare_bit_field_non_nullable_type(33)
declare_bit_field_non_nullable_type(34)
declare_bit_field_non_nullable_type(35)
declare_bit_field_non_nullable_type(36)
declare_bit_field_non_nullable_type(37)
declare_bit_field_non_nullable_type(38)
declare_bit_field_non_nullable_type(39)
declare_bit_field_non_nullable_type(40)
declare_bit_field_non_nullable_type(41)
declare_bit_field_non_nullable_type(42)
declare_bit_field_non_nullable_type(43)
declare_bit_field_non_nullable_type(44)
declare_bit_field_non_nullable_type(45)
declare_bit_field_non_nullable_type(46)
declare_bit_field_non_nullable_type(47)
declare_bit_field_non_nullable_type(48)
declare_bit_field_non_nullable_type(49)
declare_bit_field_non_nullable_type(50)
declare_bit_field_non_nullable_type(51)
declare_bit_field_non_nullable_type(52)
declare_bit_field_non_nullable_type(53)
declare_bit_field_non_nullable_type(54)
declare_bit_field_non_nullable_type(55)
declare_bit_field_non_nullable_type(56)
declare_bit_field_non_nullable_type(57)
declare_bit_field_non_nullable_type(58)
declare_bit_field_non_nullable_type(59)
declare_bit_field_non_nullable_type(60)
declare_bit_field_non_nullable_type(61)
declare_bit_field_non_nullable_type(62)
declare_bit_field_non_nullable_type(63)
declare_bit_field_non_nullable_type(64)

#define declare_uint_nullable_type(_size) \
extern data_type_info* UINT_ ## _size ## _NULLABLE;

declare_uint_nullable_type(1)
declare_uint_nullable_type(2)
declare_uint_nullable_type(3)
declare_uint_nullable_type(4)
declare_uint_nullable_type(5)
declare_uint_nullable_type(6)
declare_uint_nullable_type(7)
declare_uint_nullable_type(8)

#define declare_uint_non_nullable_type(_size) \
extern data_type_info* UINT_ ## _size ## _NON_NULLABLE;

declare_uint_non_nullable_type(1)
declare_uint_non_nullable_type(2)
declare_uint_non_nullable_type(3)
declare_uint_non_nullable_type(4)
declare_uint_non_nullable_type(5)
declare_uint_non_nullable_type(6)
declare_uint_non_nullable_type(7)
declare_uint_non_nullable_type(8)

#define declare_int_nullable_type(_size) \
extern data_type_info* INT_ ## _size ## _NULLABLE;

declare_int_nullable_type(1)
declare_int_nullable_type(2)
declare_int_nullable_type(3)
declare_int_nullable_type(4)
declare_int_nullable_type(5)
declare_int_nullable_type(6)
declare_int_nullable_type(7)
declare_int_nullable_type(8)

#define declare_int_non_nullable_type(_size) \
extern data_type_info* INT_ ## _size ## _NON_NULLABLE;

declare_int_non_nullable_type(1)
declare_int_non_nullable_type(2)
declare_int_non_nullable_type(3)
declare_int_non_nullable_type(4)
declare_int_non_nullable_type(5)
declare_int_non_nullable_type(6)
declare_int_non_nullable_type(7)
declare_int_non_nullable_type(8)

typedef long double long_double;

#define declare_float_nullable_type(size_for) \
extern data_type_info* FLOAT_ ## size_for ## _NULLABLE;

declare_float_nullable_type(float)
declare_float_nullable_type(double)
declare_float_nullable_type(long_double)

#define declare_float_non_nullable_type(size_for) \
extern data_type_info* FLOAT_ ## size_for ## _NON_NULLABLE;

declare_float_non_nullable_type(float)
declare_float_non_nullable_type(double)
declare_float_non_nullable_type(long_double)

#define declare_large_uint_nullable_type(_size) \
extern data_type_info* LARGE_UINT_ ## _size ## _NULLABLE;

declare_large_uint_nullable_type(1)
declare_large_uint_nullable_type(2)
declare_large_uint_nullable_type(3)
declare_large_uint_nullable_type(4)
declare_large_uint_nullable_type(5)
declare_large_uint_nullable_type(6)
declare_large_uint_nullable_type(7)
declare_large_uint_nullable_type(8)
declare_large_uint_nullable_type(9)
declare_large_uint_nullable_type(10)
declare_large_uint_nullable_type(11)
declare_large_uint_nullable_type(12)
declare_large_uint_nullable_type(13)
declare_large_uint_nullable_type(14)
declare_large_uint_nullable_type(15)
declare_large_uint_nullable_type(16)
declare_large_uint_nullable_type(17)
declare_large_uint_nullable_type(18)
declare_large_uint_nullable_type(19)
declare_large_uint_nullable_type(20)
declare_large_uint_nullable_type(21)
declare_large_uint_nullable_type(22)
declare_large_uint_nullable_type(23)
declare_large_uint_nullable_type(24)
declare_large_uint_nullable_type(25)
declare_large_uint_nullable_type(26)
declare_large_uint_nullable_type(27)
declare_large_uint_nullable_type(28)
declare_large_uint_nullable_type(29)
declare_large_uint_nullable_type(30)
declare_large_uint_nullable_type(31)
declare_large_uint_nullable_type(32)

#define declare_large_uint_non_nullable_type(_size) \
extern data_type_info* LARGE_UINT_ ## _size ## _NON_NULLABLE;

declare_large_uint_non_nullable_type(1)
declare_large_uint_non_nullable_type(2)
declare_large_uint_non_nullable_type(3)
declare_large_uint_non_nullable_type(4)
declare_large_uint_non_nullable_type(5)
declare_large_uint_non_nullable_type(6)
declare_large_uint_non_nullable_type(7)
declare_large_uint_non_nullable_type(8)
declare_large_uint_non_nullable_type(9)
declare_large_uint_non_nullable_type(10)
declare_large_uint_non_nullable_type(11)
declare_large_uint_non_nullable_type(12)
declare_large_uint_non_nullable_type(13)
declare_large_uint_non_nullable_type(14)
declare_large_uint_non_nullable_type(15)
declare_large_uint_non_nullable_type(16)
declare_large_uint_non_nullable_type(17)
declare_large_uint_non_nullable_type(18)
declare_large_uint_non_nullable_type(19)
declare_large_uint_non_nullable_type(20)
declare_large_uint_non_nullable_type(21)
declare_large_uint_non_nullable_type(22)
declare_large_uint_non_nullable_type(23)
declare_large_uint_non_nullable_type(24)
declare_large_uint_non_nullable_type(25)
declare_large_uint_non_nullable_type(26)
declare_large_uint_non_nullable_type(27)
declare_large_uint_non_nullable_type(28)
declare_large_uint_non_nullable_type(29)
declare_large_uint_non_nullable_type(30)
declare_large_uint_non_nullable_type(31)
declare_large_uint_non_nullable_type(32)

#define get_fixed_length_string_type(_size, _is_nullable) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = "STRING", \
	.type = STRING, \
	.is_nullable = _is_nullable, \
	.size = _size, \
	.has_variable_element_count = 0, \
	.element_count = _size, \
	.containee = UINT_1_NON_NULLABLE, \
})

#define get_fixed_length_blob_type(_size, _is_nullable) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = "BLOB", \
	.type = BLOB, \
	.is_nullable = _is_nullable, \
	.size = _size, \
	.has_variable_element_count = 0, \
	.element_count = _size, \
	.containee = UINT_1_NON_NULLABLE, \
})

#define get_variable_length_string_type(_max_size) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = "STRING", \
	.type = STRING, \
	.max_size = _max_size, \
	.has_variable_element_count = 1, \
	.containee = UINT_1_NON_NULLABLE, \
})

#define get_variable_length_blob_type(_max_size) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = "BLOB", \
	.type = BLOB, \
	.max_size = _max_size, \
	.has_variable_element_count = 1, \
	.containee = UINT_1_NON_NULLABLE, \
})

#define get_fixed_element_count_array_type(_type_name, _element_count, _max_size_if_variable_sized_element, _is_nullable_if_fixed_sized_element, _containee) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = ARRAY, \
	.is_nullable = _is_nullable_if_fixed_sized_element, \
	.max_size = _max_size_if_variable_sized_element, \
	.has_variable_element_count = 0, \
	.element_count = _element_count, \
	.containee = _containee, \
})

#define get_variable_element_count_array_type(_type_name, _element_count, _max_size, _containee) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = ARRAY, \
	.max_size = _max_size, \
	.has_variable_element_count = 1, \
	.containee = _containee, \
})

#define initialize_tuple_data_type_info(dti, _type_name, _is_nullable_if_fixed_sized_element, _max_size_if_variable_sized, _element_count) \
dti->is_static = 0; \
dti->is_finalized = 0; \
strncpy(dti->type_name, _type_name, 64); \
dti->type = TUPLE; \
dti->is_nullable = _is_nullable_if_fixed_sized_element; \
dti->max_size = _max_size_if_variable_sized; \
dti->has_variable_element_count = 0; \
dti->element_count = _element_count;

#endif