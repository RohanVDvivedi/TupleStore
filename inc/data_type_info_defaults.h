#ifndef DATA_TYPE_INFO_DEFAULTS_H
#define DATA_TYPE_INFO_DEFAULTS_H

extern data_type_info* BIT_FIELD_NULLABLE[65];

extern data_type_info* BIT_FIELD_NON_NULLABLE[65];

extern data_type_info* UINT_NULLABLE[9];

extern data_type_info* UINT_NON_NULLABLE[9];

extern data_type_info* INT_NULLABLE[9];

extern data_type_info* INT_NON_NULLABLE[9];

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

extern data_type_info* LARGE_UINT_NULLABLE[33];

extern data_type_info* LARGE_UINT_NON_NULLABLE[33];

/*
	This header and the corresponding source file provides default data_type_info and a default values to further initialize them
*/

#define define_bit_field_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = BIT_FIELD, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.bit_field_size = _size, \
})

#define define_bit_field_non_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = BIT_FIELD, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.bit_field_size = _size, \
})

#define define_uint_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = UINT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define define_uint_non_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = UINT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define define_int_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = INT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define define_int_non_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = INT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define define_float_nullable_type(_type_name, size_for) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = FLOAT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = sizeof(size_for), \
})

#define define_float_non_nullable_type(_type_name, size_for) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = FLOAT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = sizeof(size_for), \
})

#define define_large_uint_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = LARGE_UINT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define define_large_uint_non_nullable_type(_type_name, _size) \
((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = LARGE_UINT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
})

#define get_fixed_length_string_type(_type_name, _size, _is_nullable) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = STRING, \
	.is_nullable = _is_nullable, \
	.size = _size, \
	.has_variable_element_count = 0, \
	.element_count = _size, \
	.containee = UINT_NON_NULLABLE[1], \
})

#define get_fixed_length_blob_type(_size, _is_nullable) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = BLOB, \
	.is_nullable = _is_nullable, \
	.size = _size, \
	.has_variable_element_count = 0, \
	.element_count = _size, \
	.containee = UINT_NON_NULLABLE[1], \
})

#define get_variable_length_string_type(_type_name, _max_size) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = STRING, \
	.max_size = _max_size, \
	.has_variable_element_count = 1, \
	.containee = UINT_NON_NULLABLE[1], \
})

#define get_variable_length_blob_type(_type_name, _max_size) \
((data_type_info){ \
	.is_static = 0, \
	.is_finalized = 0, \
	.type_name = _type_name, \
	.type = BLOB, \
	.max_size = _max_size, \
	.has_variable_element_count = 1, \
	.containee = UINT_NON_NULLABLE[1], \
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

#define get_variable_element_count_array_type(_type_name, _max_size, _containee) \
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