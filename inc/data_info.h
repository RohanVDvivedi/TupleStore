#ifndef TUPLE_INFO_H
#define TUPLE_INFO_H

#include<stdint.h>

typedef enum data_type data_type;
enum data_type
{
	BIT_FIELD  = 0, // bit field type must never exist without a tuple or array containing them
		// BIT_FIELDs are numeric fixed bit-width unsigned data types (atmost 64 bits wide),
		// they occupy space within the null bitmap in the prefix of the tuple or array
		// hence they return size = 0

	UINT	   = 1,
	INT 	   = 2,
	FLOAT	   = 3,
	LARGE_UINT = 4,

	// above attributes are always fixed length elements

	STRING     = 5,
	BLOB       = 6,
	TUPLE      = 7,
	ARRAY      = 8,
	// the above 4 types may be fixed or variable length
	// they will be identified as container types
};

extern char type_as_string[][16];

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

	data_type_info* type_info; // -> type information of this field
};

struct data_type_info
{
	int is_static : 1; // -> if set this object is not to be freed, set for default types and custom types from extensions
	int is_finalized : 1; // -> tuple and array data_types can only be used if this is set, else call finalize on this type and get success

	char type_name[64]; // -> type name of this user defined type

	data_type type;

	int is_nullable : 1; // -> only for fixed length elements, variable length elements are always nullable by setting their corresponding offset to 0

	int is_variable_sized : 1; // -> possibly set only for STRING, BLOB, TUPLE or ARRAY
	// -> must be set if you are setting has_variable_element_count
	// -> gets derieved anyway, by the finalize function for container type hence not necessary to be set

	union
	{
		uint32_t bit_field_size; // number of bits in the bit fields, these bits need to be allocated in the prefix bitmap of the container

		uint32_t size; // -> for fixed length elements
		// -> gets derieved anyway, by the finalize function for fixed sized container types hence not necessary to be set

		uint32_t min_size; // -> for variable length elements
		// -> gets derieved anyway, by the finalize function for variable sized containers types hence not necessary to be set
	};

	uint32_t max_size; // -> for variable length elements, necessary to calculate bytes required to be allocated for offsets, sizes and counts
	// max_size may never be more than the page_size of the system

	int has_variable_element_count : 1; // -> always 0 for a TUPLE, could be 1 for an ARRAY, and also true for variable sie strings and blobs
	// must be set for variable sized string and blob

	uint32_t element_count; // -> to be used for TUPLE or ARRAY types only, and only when has_variable_element_count == 0
	// for fixed length string and blob, this must equal size

	uint32_t prefix_bitmap_size_in_bits; // -> number of bits in the prefix bitmap, valid only for tuples and fixed element count array
	// must be set to 0s for STRING and BLOB types
	// -> gets derieved anyway, by the finalize function for container types hence not necessary to be set

	data_type_info* containee;	// -> to be used for ARRAY only
	// for string and blob types the containee is always a UINT_1_NON_NULL i.e. non-nullable UINT of size 1 byte

	data_position_info containees[];	// -> to be used for TUPLE only, total element_count number of them
	// -> these data_position_infos get derieved anyway, by the finalize function hence not necessary to be set
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
#define IS_SELF(pa) (pa.positions_length == 0)																					// check if points to self
#define NEXT_POSITION(pa) ((positional_accessor){.positions_length = pa.positions_length - 1, .positions = pa.positions + 1}) 	// build a positional accessor for the next nested object
#define STATIC_POSITION(...) ((positional_accessor){ .positions_length = sizeof((uint32_t []){ __VA_ARGS__ })/sizeof(uint32_t), .positions = (uint32_t []){ __VA_ARGS__ } })
// usage STATIC_POSITION(a, b, c, d)

// varibale sized elements are always nullable
// fixed length elements and bit fields are nullable if is_nullable is set
int is_nullable_type_info(const data_type_info* dti);

// true, if is_nullable and not variable sized
int needs_is_valid_bit_in_prefix_bitmap(const data_type_info* dti);

// check if variable sized, then it will also need an offset in the container tuple or array
int is_variable_sized_type_info(const data_type_info* dti);

// get size
// size of BIT_FIELD is returned as 0, as they only occupy data in the prefix_bitmap
uint32_t get_size_for_type_info(const data_type_info* dti, const void* data);

// true only for string, blob, tuple and array
int is_container_type_info(const data_type_info* dti);

// check if variable element_count
int has_variable_element_count_for_container_type_info(const data_type_info* dti);

// get element_count
uint32_t get_element_count_for_container_type_info(const data_type_info* dti, const void* data);

// false for string and blob
// true for variable sized tuples and arrays of variable sized elements
// false also for variable element count array of fixed sized elements i.e. includes var sized strings and blobs
// this size will be total of the complete size of the data, including the size required for storing the size
int has_size_in_its_prefix_for_container_type_info(const data_type_info* dti);

// must be true for variable element count array
int has_element_count_in_its_prefix_for_container_type_info(const data_type_info* dti);

#define get_offset_to_prefix_size_for_container_type_info(dti)						(0)
#define get_bytes_required_for_prefix_size_for_container_type_info(dti) 			(get_value_size_on_page((dti)->max_size) * has_size_in_its_prefix_for_container_type_info(dti))

#define get_offset_to_prefix_element_count_for_container_type_info(dti)				(get_offset_to_prefix_size_for_container_type_info(dti) + get_bytes_required_for_prefix_size_for_container_type_info(dti))
#define get_bytes_required_for_prefix_element_count_for_container_type_info(dti) 	(get_value_size_on_page((dti)->max_size) * has_element_count_in_its_prefix_for_container_type_info(dti))

// logically equivalent to = bytes_required_for_prefix_size + bytes_required_for_prefix_element_count
#define get_offset_to_prefix_bitmap_for_container_type_info(dti)					(get_offset_to_prefix_element_count_for_container_type_info(dti) + get_bytes_required_for_prefix_element_count_for_container_type_info(dti))

// valid only for a container type info, returns the number of bits required in the prefix for the container type info
// for a tuple or a fixed element count array of fixed length elements it is equal to dti->prefix_bitmap_size_in_bits
// for an array of variable length elements it is always 0
// for a variable element count array of fixed length elements, it will be equal to element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + (dti->containee.type == BIT_FIELD) ? dti->containee.size_def.bit_field_size : 0)
uint32_t get_prefix_bitmap_size_in_bits_for_container_type_info(const data_type_info* dti, const void* data);
#define get_prefix_bitmap_size_for_container_type_info(dti, data) 					(bitmap_size_in_bytes(get_prefix_bitmap_size_in_bits_for_container_type_info(dti, data)))

// valid for string, blob, tuple and array (generated on the fly for an array, string or blob)
// valid only if index < get_element_count_for_container_type_info
data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index);
data_position_info get_data_position_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index);

// must be called and must pass on all the types
int finalize_type_info(data_type_info* dti);

// print type info
void print_type_info(data_type_info* dti);


/*
**	tuple format
**
**	variable size string/blob are just variable element count arrays of non-nullable uint8s
**
**	size -> for containers with variable sized elements (this will include variable sized tuples, but will not include variable sized strings and blobs)
**	element_count -> for containers with variable element count (this will never include tuples, but will include variable sized strings and blobs)
**	prefix_bitmap -> 1 bit for each of the field of the container that passes needs_is_valid_bit_in_prefix_bitmap(), and required bits for the BIT_FIELD elements
**	data_section -> fixed length elements stored in place, and offsets to the variable sized elements, which is followed by the actual contents of the variable sized elements
*/

/*
**		type 			variable sized element 				variable element count 				variable sized 				size in prefix 				element count in prefix
**
**		tuple 			0/1 								0									0/1 						0/1 						0
**		string/blob 	0 									0/1 								0/1 						0 							0/1
**		array0			0 									0 									0 	 						0 							0
**		array1 			0 									1 									1 							0 							1
**		array2 			1 									0 									1 							1 							0
**		array3 			1 									1 									1 							1 							1
*/

#endif