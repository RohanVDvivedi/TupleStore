#ifndef TUPLE_INFO_H
#define TUPLE_INFO_H

#include<stdint.h>

typedef enum data_type data_type;
enum data_type
{
	UINT	   = 0,
	INT 	   = 1,
	FLOAT	   = 2,
	LARGE_UINT = 3,
	BIT_FIELD  = 4, // bit field typemay never exist without a tuple or array containing them
		// BIT_FIELDs are numeric fixed bit-width unsigned data types,
		// they occupy space within the null bitmap in the prefix of the tuple, if any
	// numeric datatypes end

	// above attributes are always fixed length elements

	STRING     = 5,
	BLOB       = 6,
	TUPLE      = 7,
	ARRAY      = 8,
	// the above 4 types may be fixed or variable length
};

typedef struct data_size_info data_size_info;
struct data_size_info
{
	int is_variable_sized : 1; // -> possibly set only for STRING, BLOB, TUPLE or ARRAY

	union
	{
		uint32_t size; // -> for fixed length elements

		uint32_t min_size; // -> for variable length elements

		uint32_t bit_field_size; // number of bits in the bit fields, these bits need to be allocated in the prefix bitmap of the container
	};
};

typedef struct data_type_info data_type_info;

typedef struct data_position_info data_position_info;
struct data_position_info
{
	char field_name[64]; // -> field name of this field

	union
	{
		uint32_t byte_offset; // -> for fixed length fields

		uint32_t byte_offset_to_byte_offset; // -> for variable length fields

		uint32_t bit_offset_in_prefix_bitmap; // -> for bitfields
	};

	uint32_t bit_offset_to_is_valid_bit; // -> for bit fields and fixed length elements only, and only when they have is_nullable bit set in the type_info

	data_type_info* type_info; // type information of this field
};

struct data_type_info
{
	char type_name[64]; // -> type name of this user defined type

	data_type type;

	int is_nullable : 1; // -> only for fixed length elements, variable length elements are always nullable by setting their corresponding offset to 0

	data_size_info size_info; // the only struct required by the functions of page_layout

	uint32_t max_size; // -> for variable length elements, necessary to calculate bytes to allocate for offsets, sizes and counts
	// max_size may never be more than the page_size of the system

	int has_variable_element_count : 1; // -> always 0 for a TUPLE, could be 1 for an ARRAY

	uint32_t element_count; // -> to be used for TUPLE or ARRAY types only, and only when has_variable_element_count == 0

	data_type_info* containee;	// -> to be used for ARRAY only

	data_position_info containees[];	// -> to be used for TUPLE only, total element_count number of them
};

// used to access nested data like TUPLE, ARRAY, STRING and BLOB only
// stores the complete path to access the object
typedef struct positional_accessor positional_accessor;
struct positional_accessor
{
	uint32_t positions_length;

	uint32_t* positions;
};

// for a given positional accessor the nest child to work on is at index positions[0], if the positions_length > 0

// utiities for positional accessors
#define SELF ((positional_accessor){.positions_length = 0, .positions = NULL})													// point to self
#define IS_SELF(pa) (pa.positions_length == 0);																					// check if points to self
#define NEXT_POSITION(pa) ((positional_accessor){.positions_length = pa.positions_length - 1, .positions = pa.positions + 1}) 	// build a positional accessor for the next nested object
#define STATIC_POSITION(...) ((positional_accessor){ .positions_length = sizeof((uint32_t []){ __VA_ARGS__ })/sizeof(uint32_t), .positions = (uint32_t []){ __VA_ARGS__ } })
// usage STATIC_POSITION(a, b, c, d)

// varibale sized elements are always nullable
// fixed length elements and bit fields are nullable if is_nullable is set
int is_nullable_type_info(const data_type_info* dti);

// true, if is_nullable and not variable sized
int needs_bit_in_is_valid_bitmap(const data_type_info* dti);

// check if variable sized, then it will also need an offset in the container tuple or array
int is_variable_sized_size_info(const data_size_info* dsi);
int is_variable_sized_type_info(const data_type_info* dti);

// get size
uint32_t get_element_size(const data_type_info* dti, const void* data);

// check if variable element_count, only possible for an array, string and blob
int has_variable_element_count(const data_type_info* dti);

// get element_count
uint32_t get_element_count(const data_type_info* dti, const void* data);

// true for variable sized string, blob, tuple and array
// this size will be total of the complete size of the data, including the size required for storing the size
int has_size_in_its_prefix(const data_type_info* dti);

// true for variable element count array containing variable sized elements
int has_element_count_in_its_prefix(const data_type_info* dti);

#define get_bytes_required_for_prefix_size(dti) 			get_value_size_on_page(dti->max_size)
#define get_bytes_required_for_prefix_element_count(dti) 	get_value_size_on_page(dti->max_size)

// true for string, blob, tuple and array
int is_container_type(const data_type_info* dti);

// valid for string, blob, tuple and array (generated on the fly for an array)
data_position_info get_data_position_info_for_container(const data_type_info* dti, const void* data, uint32_t index);

#endif