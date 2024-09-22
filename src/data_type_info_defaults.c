#include<data_type_info.h>
#include<data_type_info_defaults.h>

#define define_bit_field_nullable_type(_size) \
data_type_info* BIT_FIELD_ ## _size ## _NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "BIT_FIELD", \
	.type = BIT_FIELD, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.bit_field_size = _size, \
});

define_bit_field_nullable_type(1)
define_bit_field_nullable_type(2)
define_bit_field_nullable_type(3)
define_bit_field_nullable_type(4)
define_bit_field_nullable_type(5)
define_bit_field_nullable_type(6)
define_bit_field_nullable_type(7)
define_bit_field_nullable_type(8)
define_bit_field_nullable_type(9)
define_bit_field_nullable_type(10)
define_bit_field_nullable_type(11)
define_bit_field_nullable_type(12)
define_bit_field_nullable_type(13)
define_bit_field_nullable_type(14)
define_bit_field_nullable_type(15)
define_bit_field_nullable_type(16)
define_bit_field_nullable_type(17)
define_bit_field_nullable_type(18)
define_bit_field_nullable_type(19)
define_bit_field_nullable_type(20)
define_bit_field_nullable_type(21)
define_bit_field_nullable_type(22)
define_bit_field_nullable_type(23)
define_bit_field_nullable_type(24)
define_bit_field_nullable_type(25)
define_bit_field_nullable_type(26)
define_bit_field_nullable_type(27)
define_bit_field_nullable_type(28)
define_bit_field_nullable_type(29)
define_bit_field_nullable_type(30)
define_bit_field_nullable_type(31)
define_bit_field_nullable_type(32)
define_bit_field_nullable_type(33)
define_bit_field_nullable_type(34)
define_bit_field_nullable_type(35)
define_bit_field_nullable_type(36)
define_bit_field_nullable_type(37)
define_bit_field_nullable_type(38)
define_bit_field_nullable_type(39)
define_bit_field_nullable_type(40)
define_bit_field_nullable_type(41)
define_bit_field_nullable_type(42)
define_bit_field_nullable_type(43)
define_bit_field_nullable_type(44)
define_bit_field_nullable_type(45)
define_bit_field_nullable_type(46)
define_bit_field_nullable_type(47)
define_bit_field_nullable_type(48)
define_bit_field_nullable_type(49)
define_bit_field_nullable_type(50)
define_bit_field_nullable_type(51)
define_bit_field_nullable_type(52)
define_bit_field_nullable_type(53)
define_bit_field_nullable_type(54)
define_bit_field_nullable_type(55)
define_bit_field_nullable_type(56)
define_bit_field_nullable_type(57)
define_bit_field_nullable_type(58)
define_bit_field_nullable_type(59)
define_bit_field_nullable_type(60)
define_bit_field_nullable_type(61)
define_bit_field_nullable_type(62)
define_bit_field_nullable_type(63)
define_bit_field_nullable_type(64)

#define define_bit_field_non_nullable_type(_size) \
data_type_info* BIT_FIELD_ ## _size ## _NON_NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "BIT_FIELD", \
	.type = BIT_FIELD, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.bit_field_size = _size, \
});

define_bit_field_non_nullable_type(1)
define_bit_field_non_nullable_type(2)
define_bit_field_non_nullable_type(3)
define_bit_field_non_nullable_type(4)
define_bit_field_non_nullable_type(5)
define_bit_field_non_nullable_type(6)
define_bit_field_non_nullable_type(7)
define_bit_field_non_nullable_type(8)
define_bit_field_non_nullable_type(9)
define_bit_field_non_nullable_type(10)
define_bit_field_non_nullable_type(11)
define_bit_field_non_nullable_type(12)
define_bit_field_non_nullable_type(13)
define_bit_field_non_nullable_type(14)
define_bit_field_non_nullable_type(15)
define_bit_field_non_nullable_type(16)
define_bit_field_non_nullable_type(17)
define_bit_field_non_nullable_type(18)
define_bit_field_non_nullable_type(19)
define_bit_field_non_nullable_type(20)
define_bit_field_non_nullable_type(21)
define_bit_field_non_nullable_type(22)
define_bit_field_non_nullable_type(23)
define_bit_field_non_nullable_type(24)
define_bit_field_non_nullable_type(25)
define_bit_field_non_nullable_type(26)
define_bit_field_non_nullable_type(27)
define_bit_field_non_nullable_type(28)
define_bit_field_non_nullable_type(29)
define_bit_field_non_nullable_type(30)
define_bit_field_non_nullable_type(31)
define_bit_field_non_nullable_type(32)
define_bit_field_non_nullable_type(33)
define_bit_field_non_nullable_type(34)
define_bit_field_non_nullable_type(35)
define_bit_field_non_nullable_type(36)
define_bit_field_non_nullable_type(37)
define_bit_field_non_nullable_type(38)
define_bit_field_non_nullable_type(39)
define_bit_field_non_nullable_type(40)
define_bit_field_non_nullable_type(41)
define_bit_field_non_nullable_type(42)
define_bit_field_non_nullable_type(43)
define_bit_field_non_nullable_type(44)
define_bit_field_non_nullable_type(45)
define_bit_field_non_nullable_type(46)
define_bit_field_non_nullable_type(47)
define_bit_field_non_nullable_type(48)
define_bit_field_non_nullable_type(49)
define_bit_field_non_nullable_type(50)
define_bit_field_non_nullable_type(51)
define_bit_field_non_nullable_type(52)
define_bit_field_non_nullable_type(53)
define_bit_field_non_nullable_type(54)
define_bit_field_non_nullable_type(55)
define_bit_field_non_nullable_type(56)
define_bit_field_non_nullable_type(57)
define_bit_field_non_nullable_type(58)
define_bit_field_non_nullable_type(59)
define_bit_field_non_nullable_type(60)
define_bit_field_non_nullable_type(61)
define_bit_field_non_nullable_type(62)
define_bit_field_non_nullable_type(63)
define_bit_field_non_nullable_type(64)

#define define_uint_nullable_type(_size) \
data_type_info* UINT_ ## _size ## _NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "UINT", \
	.type = UINT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_uint_nullable_type(1)
define_uint_nullable_type(2)
define_uint_nullable_type(3)
define_uint_nullable_type(4)
define_uint_nullable_type(5)
define_uint_nullable_type(6)
define_uint_nullable_type(7)
define_uint_nullable_type(8)

#define define_uint_non_nullable_type(_size) \
data_type_info* UINT_ ## _size ## _NON_NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "UINT", \
	.type = UINT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_uint_non_nullable_type(1)
define_uint_non_nullable_type(2)
define_uint_non_nullable_type(3)
define_uint_non_nullable_type(4)
define_uint_non_nullable_type(5)
define_uint_non_nullable_type(6)
define_uint_non_nullable_type(7)
define_uint_non_nullable_type(8)

#define define_int_nullable_type(_size) \
data_type_info* INT_ ## _size ## _NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "INT", \
	.type = INT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_int_nullable_type(1)
define_int_nullable_type(2)
define_int_nullable_type(3)
define_int_nullable_type(4)
define_int_nullable_type(5)
define_int_nullable_type(6)
define_int_nullable_type(7)
define_int_nullable_type(8)

#define define_int_non_nullable_type(_size) \
data_type_info* INT_ ## _size ## _NON_NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "INT", \
	.type = INT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_int_non_nullable_type(1)
define_int_non_nullable_type(2)
define_int_non_nullable_type(3)
define_int_non_nullable_type(4)
define_int_non_nullable_type(5)
define_int_non_nullable_type(6)
define_int_non_nullable_type(7)
define_int_non_nullable_type(8)

#define define_float_nullable_type(size_for) \
data_type_info* FLOAT_ ## size_for ## _NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "FLOAT", \
	.type = FLOAT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = sizeof(size_for), \
});

define_float_nullable_type(float)
define_float_nullable_type(double)
define_float_nullable_type(long_double)

#define define_float_non_nullable_type(size_for) \
data_type_info* FLOAT_ ## size_for ## _NON_NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "FLOAT", \
	.type = FLOAT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = sizeof(size_for), \
});

define_float_non_nullable_type(float)
define_float_non_nullable_type(double)
define_float_non_nullable_type(long_double)

#define define_large_uint_nullable_type(_size) \
data_type_info* LARGE_UINT_ ## _size ## _NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "LARGE_UINT", \
	.type = LARGE_UINT, \
	.is_nullable = 1, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_large_uint_nullable_type(1)
define_large_uint_nullable_type(2)
define_large_uint_nullable_type(3)
define_large_uint_nullable_type(4)
define_large_uint_nullable_type(5)
define_large_uint_nullable_type(6)
define_large_uint_nullable_type(7)
define_large_uint_nullable_type(8)
define_large_uint_nullable_type(9)
define_large_uint_nullable_type(10)
define_large_uint_nullable_type(11)
define_large_uint_nullable_type(12)
define_large_uint_nullable_type(13)
define_large_uint_nullable_type(14)
define_large_uint_nullable_type(15)
define_large_uint_nullable_type(16)
define_large_uint_nullable_type(17)
define_large_uint_nullable_type(18)
define_large_uint_nullable_type(19)
define_large_uint_nullable_type(20)
define_large_uint_nullable_type(21)
define_large_uint_nullable_type(22)
define_large_uint_nullable_type(23)
define_large_uint_nullable_type(24)
define_large_uint_nullable_type(25)
define_large_uint_nullable_type(26)
define_large_uint_nullable_type(27)
define_large_uint_nullable_type(28)
define_large_uint_nullable_type(29)
define_large_uint_nullable_type(30)
define_large_uint_nullable_type(31)
define_large_uint_nullable_type(32)

#define define_large_uint_non_nullable_type(_size) \
data_type_info* LARGE_UINT_ ## _size ## _NON_NULLABLE = &((data_type_info){ \
	.is_static = 1, \
	.is_finalized = 0, \
	.type_name = "LARGE_UINT", \
	.type = LARGE_UINT, \
	.is_nullable = 0, \
	.is_variable_sized = 0, \
	.size = _size, \
});

define_large_uint_non_nullable_type(1)
define_large_uint_non_nullable_type(2)
define_large_uint_non_nullable_type(3)
define_large_uint_non_nullable_type(4)
define_large_uint_non_nullable_type(5)
define_large_uint_non_nullable_type(6)
define_large_uint_non_nullable_type(7)
define_large_uint_non_nullable_type(8)
define_large_uint_non_nullable_type(9)
define_large_uint_non_nullable_type(10)
define_large_uint_non_nullable_type(11)
define_large_uint_non_nullable_type(12)
define_large_uint_non_nullable_type(13)
define_large_uint_non_nullable_type(14)
define_large_uint_non_nullable_type(15)
define_large_uint_non_nullable_type(16)
define_large_uint_non_nullable_type(17)
define_large_uint_non_nullable_type(18)
define_large_uint_non_nullable_type(19)
define_large_uint_non_nullable_type(20)
define_large_uint_non_nullable_type(21)
define_large_uint_non_nullable_type(22)
define_large_uint_non_nullable_type(23)
define_large_uint_non_nullable_type(24)
define_large_uint_non_nullable_type(25)
define_large_uint_non_nullable_type(26)
define_large_uint_non_nullable_type(27)
define_large_uint_non_nullable_type(28)
define_large_uint_non_nullable_type(29)
define_large_uint_non_nullable_type(30)
define_large_uint_non_nullable_type(31)
define_large_uint_non_nullable_type(32)