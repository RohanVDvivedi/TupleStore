#ifndef DATA_TYPE_INFO_H
#define DATA_TYPE_INFO_H

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

extern char types_as_string[][16];

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
	int is_static; // -> if set this object is not to be freed, set for default types and custom types from extensions
	int is_finalized; // -> tuple and array data_types can only be used if this is set, else call finalize on this type and get success

	char type_name[64]; // -> type name of this user defined type, equivalent to a typedef in C

	data_type type;

	int is_nullable; // -> only for fixed length elements, variable length elements are always nullable by setting their corresponding offset to 0

	int is_variable_sized; // -> possibly set only for STRING, BLOB, TUPLE or ARRAY
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

	int has_variable_element_count; // -> always 0 for a TUPLE, could be 1 for an ARRAY, and also true for variable sie strings and blobs
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

// macro to calculate sizeof tuple data_type_info
#define sizeof_tuple_data_type_info(element_count) (sizeof(data_type_info) + sizeof(data_position_info) * (element_count))

#include<data_type_info_defaults.h>

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

// NOTE:: call this function only if you know whta you are doing
// if the container has size in prefix, then it is overwritten with the new provided size
// this call must exist only in this library and must not exist in the user code
int overwrite_size_for_container_type_info_with_size_in_prefix(const data_type_info* dti, void* data, uint32_t new_size);

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

// returns NULL, if the index is definitely out of bounds (this check is performed only if it is a fixed element count container), or if you are attempting to index a non-container data_type_info
// no checks to ensure that index is within bounds is done for variable sized strings, variable sized blobs OR variable element count arrays
data_type_info* get_data_type_info_for_containee_of_container_without_data(const data_type_info* dti, uint32_t index);

// valid for string, blob, tuple and array (generated on the fly for an array, string or blob)
// valid only if index < get_element_count_for_container_type_info
data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index);
data_position_info get_data_position_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index);

// finds a containee's index using its field_name
// if it fails, it will return -1 i.e. UINT32_MAX 
uint32_t find_containee_using_field_name_in_tuple_type_info(const data_type_info* dti, const char* field_name);

// must be called and must pass on all the types
int finalize_type_info(data_type_info* dti);

uint32_t get_byte_count_for_serialized_type_info(const data_type_info* dti); // returns number of bytes in serialized type_info
uint32_t serialize_type_info(const data_type_info* dti, void* data); // data must be able to hold atleast enough bytes to serialize dti, you may get this size by calling get_byte_count_for_serialized_type_info first
data_type_info* deserialize_type_info(const void* data, uint32_t data_size, int* allocation_error); // this function fails if we require more bytes than data_size for deserialization, it will fail with a NULL
// if the returned value is NULL, and allocation_error is set, then the failure happenned because we were unable to allocate a data_type_info node using the malloc std c allocator
// for the return value of this function you may destroy all nodes recursively except the ones marked is_static = 1
// after deserialize_type_info returns success, you still need to call finalize_type_info on the output to make it any usefull

// destroys all non-static (is_static = 0) data_type_info recursively
// it is a candidate function to destroy the output of deserialize_type_info
void destroy_non_static_type_info_recursively(data_type_info* dti);

// returns 1, if the 2 data_type_info are logically identical
int are_identical_type_info(const data_type_info* dti1, const data_type_info* dti2);

// print type info
void print_type_info(const data_type_info* dti);


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

#include<user_value.h>

/*
	primitive types like BIT_FIELD, UINT, INT, FLOAT, LARGE_UINT, STRING, BLOB, can be compared and set with each other without regard to their size or their variability of their size
	i.e. a UINT of size 1 can be set with a uservalue pointing to UINT of size 3, this might result in soem data loss but it is assumes that you know what you are doing
		similarly, if you data_type_info-s have type = STRING, then they can be set even if they both have different fixed sizes, OR 1 being variable sized and another being fixed size, there can be data loss here

	for TUPLE and ARRAY we always assume that the data_type_info of the user_value is exactly same as that of data, even if they are variable sized, then even their max_size must not be different
	i.e. if they are variable sized their max_size also can not be different, neither can they have different containee/s
	TUPLE and ARRAY types are literally copied from their user_values
*/

int is_containee_null_in_container(const data_type_info* dti, const void* data, uint32_t index);

// returns pointer to the value from the container, if the containee is a BIT_FIELD then we return the pointer to the prefix_bitmap where it resides
const void* get_pointer_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index);

// for fixed length elements it is same as dti->size
// for variable length elements, it is 0 if they are NULL
// else we get their size from their pointer
uint32_t get_size_of_containee_from_container(const data_type_info* dti, const void* data, uint32_t index);

// returns NULL by default for BIT_FIELD types, as bit_fields must exist inside a container
const user_value get_user_value_for_type_info(const data_type_info* dti, const void* data);

const user_value get_user_value_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index);

int is_variable_sized_containee_at_end_of_container(const data_type_info* dti, void* data, uint32_t index);

// this needs to be done for modifying the variable sized element of the container, if its size would change
// if the above function passes, we do not need to call this function
int move_variable_sized_containee_to_end_of_container(const data_type_info* dti, void* data, uint32_t index);

// initializes the 0-ed out data with minimum size for this element
// this is a NO-OP for bitfield elements
// returns the size of the data
// it is equivalent to set_user_value_for_type_info(dti, data, 0, dti->min_size, EMPTY_USER_VALUE);
uint32_t initialize_minimal_data_for_type_info(const data_type_info* dti, void* data);

// returns true, if the data is minimal initialized or set to EMPTY_USER_VALUE
// if this function returns 1, this implies data is equivalent to EMPTY_USER_VALUE
int is_minimal_data_for_type_info(const data_type_info* dti, const void* data);

int set_containee_to_NULL_in_container(const data_type_info* dti, void* data, uint32_t index);

// data here is assumed to contain garbage
// max_size_increment_allowed is never a problem for fixed length elements
// if is_valid bit is set, then the data is valid valud with data_type_info
int can_set_user_value_for_type_info(const data_type_info* dti, const void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR the dti is BIT_FIELD type OR uval is NULL
// max_size_increment_allowed is never a problem for fixed length elements
// if is_valid bit is set, then the data is valid valud with data_type_info
int set_user_value_for_type_info(const data_type_info* dti, void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR uval is NULL and the element is non-NULLABLE
// max_size_increment_allowed is never a problem for fixed length elements
int can_set_user_value_to_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR uval is NULL and the element is non-NULLABLE
// max_size_increment_allowed is never a problem for fixed length elements
int set_user_value_to_containee_in_container(const data_type_info* dti, void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR if index is not within 0 and element_count
// check to see if the below function will succeed
int can_expand_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

// returns 0, if the max_size_increment is violated OR if index is not within 0 and element_count
// added slots number of slots at the given index, if the index is equal to element count then the slots are added at the end
// the new slots are either left NULL or 0, which ever is minimal work
int expand_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

// returns 0, if index is out of bounds OR there are not enough slots after first index slots
// check to see if the below function will succeed
int can_discard_from_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots);

// returns 0, if index is out of bounds OR there are not enough slots after first index slots
// discard slots number of slots starting at index location, index muts be between 0 and (element_count - 1)
int discard_from_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots);

// can not be used for BIT_FIELDs
void print_data_for_data_type_info(const data_type_info* dti, const void* data);





#include<primitive_numeral_types.h>

// comparision utility

// hashing utility

// does not work for BIT_FIELD
// non containers are hashed directly, containers are hashed element by element, and XORed together
// so STRING, variable sized STRING and an ARRAY of UINT_NON_NULLABLE[1] with same content hash to the same value
uint64_t hash_data_for_type_info(const data_type_info* dti, const void* data, uint64_t (*hash_func)(const void* data, uint32_t size));

uint64_t hash_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint64_t (*hash_func)(const void* data, uint32_t size));

#endif