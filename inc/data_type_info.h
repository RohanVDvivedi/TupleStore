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

	PRIMITIVE_TYPES_NUMBERING_LAST = 4,
	// above attributes are always fixed length elements

	STRING     = 5,
	BLOB       = 6,
	TUPLE      = 7,
	ARRAY      = 8,
	// the above 4 types may be fixed or variable length
	// they will be identified as container types
};
#define is_container_data_type(type) ((type) > PRIMITIVE_TYPES_NUMBERING_LAST)

extern char types_as_string[][16];

typedef struct data_type_info data_type_info;

typedef struct data_positional_info data_positional_info;
struct data_positional_info
{
	union
	{
		uint32_t byte_offset; // -> for fixed length fields

		uint32_t byte_offset_to_byte_offset; // -> for variable length fields

		uint32_t bit_offset_in_prefix_bitmap; // -> for bitfields
	};

	uint32_t bit_offset_to_is_valid_bit; // -> for bit fields and fixed length elements only, and only when they have is_nullable bit set in the type_info

	data_type_info* type_info; // -> type information of this field
};

// data_positional_info belongs to a container inside a container
// it must have a non-null type_info, unless the index is out of bounds
#define INVALID_DATA_POSITIONAL_INFO ((data_positional_info){.type_info = NULL})
#define IS_INVALID_DATA_POSITIONAL_INFO(pos_info) ((pos_info)->type_info == NULL)

typedef struct data_position_info data_position_info;
struct data_position_info
{
	char field_name[64]; // -> field name of this field

	data_positional_info al; // -> information about the actual position inthe tuple or array
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
static inline int is_nullable_type_info(const data_type_info* dti);

// true, if is_nullable and not variable sized
static inline int needs_is_valid_bit_in_prefix_bitmap(const data_type_info* dti);

// check if variable sized, then it will also need an offset in the container tuple or array
static inline int is_variable_sized_type_info(const data_type_info* dti);

// get size
// size of BIT_FIELD is returned as 0, as they only occupy data in the prefix_bitmap
static inline uint32_t get_size_for_type_info(const data_type_info* dti, const void* data);

// NOTE:: call this function only if you know whta you are doing
// if the container has size in prefix, then it is overwritten with the new provided size
// this call must exist only in this library and must not exist in the user code
static inline int overwrite_size_for_container_type_info_with_size_in_prefix(const data_type_info* dti, void* data, uint32_t new_size);

// true only for string, blob, tuple and array
static inline int is_container_type_info(const data_type_info* dti);

// check if variable element_count
static inline int is_variable_element_count_container_type_info(const data_type_info* dti);

// get element_count
static inline uint32_t get_element_count_for_container_type_info(const data_type_info* dti, const void* data);

// false for string and blob
// true for variable sized tuples and arrays of variable sized elements
// false also for variable element count array of fixed sized elements i.e. includes var sized strings and blobs
// this size will be total of the complete size of the data, including the size required for storing the size
static inline int has_size_in_its_prefix_for_container_type_info(const data_type_info* dti);

// must be true for variable element count array
static inline int has_element_count_in_its_prefix_for_container_type_info(const data_type_info* dti);

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
static inline uint32_t get_prefix_bitmap_size_in_bits_for_container_type_info(const data_type_info* dti, const void* data);
#define get_prefix_bitmap_size_for_container_type_info(dti, data) 					(bitmap_size_in_bytes(get_prefix_bitmap_size_in_bits_for_container_type_info(dti, data)))

/*
	for all the functions that act on the containee inside the container, take index as input
	these functions most of which have a *_CONTAINITY_USSAFE version that do not check that the element_count is within bounds
	you may use their these versions if you are sure that you will not need to make checks like
	the dti passed being a container and the element_count being within bounds
*/

// returns NULL, if the index is definitely out of bounds (this check is performed only if it is a fixed element count container), or if you are attempting to index a non-container data_type_info
// no checks to ensure that index is within bounds is done for variable sized strings, variable sized blobs OR variable element count arrays
static inline data_type_info* get_data_type_info_for_containee_of_container_without_data(const data_type_info* dti, uint32_t index);

// valid for string, blob, tuple and array (generated on the fly for an array, string or blob)
// valid only if index < get_element_count_for_container_type_info
static inline data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index);
// below function is evaluated only if the passed cached_return is INVALID_DATA_POSITION_INFO, else it is a NO-OP
static inline int get_data_positional_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* cached_return); // always prefer to use this over data_position_info

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

/*
	For all the functions below that take data_positional_info* containee_pos_info, this parameter is the cached prior copy of the accessible element inside the container (array or tuple or string or blob)
	if you do not have ithis information prior to making this call pass a pointer to a local variable that is set to INVALID_DATA_POSITIONAL_INFO
*/

static inline int is_containee_null_in_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info);

// returns pointer to the value from the container, if the containee is a BIT_FIELD then we return the pointer to the prefix_bitmap where it resides
// for a NULL value a c compatible NULL is returned, hence you may not need to call is_containee_null_in_container, post this call
static inline const void* get_pointer_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info);

// for fixed length elements it is same as dti->size
// for variable length elements, it is 0 if they are NULL
// else we get their size from their pointer
static inline uint32_t get_size_of_containee_from_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info);

// returns NULL by default for BIT_FIELD types, as bit_fields must exist inside a container
static inline int get_user_value_for_type_info(user_value* uval, const data_type_info* dti, const void* data);

static inline int get_user_value_to_containee_from_container(user_value* uval, const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info);

// this needs to be done for modifying the variable sized element of the container, if its size would change
// if the above function passes, we do not need to call this function
static inline int move_variable_sized_containee_to_end_of_container(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info);

// initializes the 0-ed out data with minimum size for this element
// this is a NO-OP for bitfield elements
// returns the size of the data
// it is equivalent to set_user_value_for_type_info(dti, data, 0, dti->min_size, EMPTY_USER_VALUE);
static inline uint32_t initialize_minimal_data_for_type_info(const data_type_info* dti, void* data);

// returns true, if the data is minimal initialized or set to EMPTY_USER_VALUE
// if this function returns 1, this implies data is equivalent to EMPTY_USER_VALUE
static inline int is_minimal_data_for_type_info(const data_type_info* dti, const void* data);

static inline int set_containee_to_NULL_in_container(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info);

// data here is assumed to contain garbage
// max_size_increment_allowed is never a problem for fixed length elements
// if is_valid bit is set, then the data is valid valud with data_type_info
static inline int can_set_user_value_for_type_info(const data_type_info* dti, const void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR the dti is BIT_FIELD type OR uval is NULL
// max_size_increment_allowed is never a problem for fixed length elements
// if is_valid bit is set, then the data is valid valud with data_type_info
static inline int set_user_value_for_type_info(const data_type_info* dti, void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval);

// returns 0, if the max_size_increment is violated OR uval is NULL and the element is non-NULLABLE
// max_size_increment_allowed is never a problem for fixed length elements
static inline int can_set_user_value_to_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval, data_positional_info* containee_pos_info);

// returns 0, if the max_size_increment is violated OR uval is NULL and the element is non-NULLABLE
// max_size_increment_allowed is never a problem for fixed length elements
static inline int set_user_value_to_containee_in_container(const data_type_info* dti, void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval, data_positional_info* containee_pos_info);

// returns 0, if the max_size_increment is violated OR if index is not within 0 and element_count
// check to see if the below function will succeed
static inline int can_expand_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

// returns 0, if the max_size_increment is violated OR if index is not within 0 and element_count
// added slots number of slots at the given index, if the index is equal to element count then the slots are added at the end
// the new slots are either left NULL or 0, which ever is minimal work
static inline int expand_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed);

// returns 0, if index is out of bounds OR there are not enough slots after first index slots
// check to see if the below function will succeed
static inline int can_discard_from_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots);

// returns 0, if index is out of bounds OR there are not enough slots after first index slots
// discard slots number of slots starting at index location, index muts be between 0 and (element_count - 1)
static inline int discard_from_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots);

// can not be used for BIT_FIELDs
void print_data_for_data_type_info(const data_type_info* dti, const void* data);





#include<primitive_numeral_types.h>

// comparision utility

// hashing utility

#include<tuple_hasher.h>

// does not work for BIT_FIELD
// non containers are hashed directly, containers are hashed element by element, and XORed together
// so STRING, variable sized STRING and an ARRAY of UINT_NON_NULLABLE[1] with same content hash to the same value
uint64_t hash_data_for_type_info(const data_type_info* dti, const void* data, tuple_hasher* th);

uint64_t hash_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, tuple_hasher* th, data_positional_info* containee_pos_info);


// static inline implementation

#include<page_layout_util.h>

#include<serial_int.h>
#include<large_uints.h>
#include<float_accesses.h>

#include<bitmap.h>

#include<string.h>
#include<stdio.h>

static inline int is_nullable_type_info(const data_type_info* dti)
{
	return is_variable_sized_type_info(dti) || dti->is_nullable;
}

static inline int needs_is_valid_bit_in_prefix_bitmap(const data_type_info* dti)
{
	return (!is_variable_sized_type_info(dti)) && dti->is_nullable;
}

static inline int is_variable_sized_type_info(const data_type_info* dti)
{
	return dti->is_variable_sized;
}

static inline uint32_t get_size_for_type_info(const data_type_info* dti, const void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
		return dti->size;

	// variable sized element either has its size in prefix or its element count in the prefix or both
	if(has_size_in_its_prefix_for_container_type_info(dti))
	{
		uint32_t size = read_value_from_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size);
		return (size == 0) ? dti->max_size : size; // a variable sized element is never 0 sized (because it is storing size in prefix), it is probably max_size
	}

	uint32_t element_count = read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size);

	// now we know for sure that this is variable sized container, but without size in its prefix
	// so this must be a container precisely : variable sized string, variable sized blob or array of variable element count but of fixed length type
	// all in all we know the element_count and that each element is fixed sized element
	
	if(dti->containee->type == BIT_FIELD)
		return get_value_size_on_page(dti->max_size) // it has element_count in its prefix but not its size
		 + bitmap_size_in_bytes(element_count * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size));
	else
		return get_value_size_on_page(dti->max_size) // it has element_count in its prefix but not its size
		 + bitmap_size_in_bytes(element_count * needs_is_valid_bit_in_prefix_bitmap(dti->containee))
		 + (element_count * dti->containee->size);
}

static inline int overwrite_size_for_container_type_info_with_size_in_prefix(const data_type_info* dti, void* data, uint32_t new_size)
{
	if(!has_size_in_its_prefix_for_container_type_info(dti))
		return 0;

	if(new_size > dti->max_size)
		return 0;

	write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);

	return 1;
}

static inline int is_container_type_info(const data_type_info* dti)
{
	return is_container_data_type(dti->type);
}

// check if variable element_count
static inline int is_variable_element_count_container_type_info(const data_type_info* dti)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// not possible for fixed sized element
	if(!is_variable_sized_type_info(dti))
		return 0;

	// tuples are by default fixed element_count, yet they could be variable sized
	if(dti->type == TUPLE)
		return 0;

	// a variable sized string or blob is always variable element count container
	if(dti->type == STRING || dti->type == BLOB)
		return 1;

	// this leaves us with variable sized strings, blobs and arrays with either variable element count or containing variable sized elements
	return dti->has_variable_element_count;
}

static inline uint32_t get_element_count_for_container_type_info(const data_type_info* dti, const void* data)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// if not variable element_count then read from the type_info
	if(!is_variable_element_count_container_type_info(dti))
		return dti->element_count;

	// else read from the data
	return read_value_from_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size);
}

static inline int has_size_in_its_prefix_for_container_type_info(const data_type_info* dti)
{
	// not possible for a non-container
	if(!is_container_type_info(dti))
		return 0;

	// not possible for fixed sized element
	if(!is_variable_sized_type_info(dti))
		return 0;

	// size must be stored for variable sized tuples
	if(dti->type == TUPLE)
		return 1;

	// not required for a variable sized string or blob
	// we are already storing element_count for them
	if(dti->type == STRING || dti->type == BLOB)
		return 0;

	// for an array we need to store the total size only if the individual elements that it contains is variable sized
	return is_variable_sized_type_info(dti->containee);
}

static inline int has_element_count_in_its_prefix_for_container_type_info(const data_type_info* dti)
{
	return is_variable_element_count_container_type_info(dti);
}

static inline uint32_t get_prefix_bitmap_size_in_bits_for_container_type_info(const data_type_info* dti, const void* data)
{
	// no prefix bitmap for non container type data_types
	if(!is_container_type_info(dti))
		return 0;

	// for tuple used the cached value
	if(dti->type == TUPLE)
		return dti->prefix_bitmap_size_in_bits;

	// no prefix bitmap necessary for STRING and BLOB
	if(dti->type == STRING || dti->type == BLOB)
		return 0;

	// this must now be an array

	if(dti->containee->type == BIT_FIELD)
		return get_element_count_for_container_type_info(dti, data) * (needs_is_valid_bit_in_prefix_bitmap(dti->containee) + dti->containee->bit_field_size);
	else
		return get_element_count_for_container_type_info(dti, data) * needs_is_valid_bit_in_prefix_bitmap(dti->containee);
}

static inline data_type_info* get_data_type_info_for_containee_of_container_without_data(const data_type_info* dti, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return NULL;

	// for fixed element count containers make sure that index is without bounds
	if(!is_variable_element_count_container_type_info(dti) && index >= dti->element_count)
		return NULL;

	// proceed as now the index is probably within bounds

	if(dti->type == TUPLE)
		return dti->containees[index].al.type_info;

	if(dti->type == STRING || dti->type == BLOB)
		return UINT_NON_NULLABLE[1]; // this must be the containee here, so why not return the default

	// else it has to be an array
	return dti->containee;
}

static inline data_type_info* get_data_type_info_for_containee_of_container_CONTAINITY_UNSAFE(const data_type_info* dti, const void* data, uint32_t index)
{
	if(dti->type == TUPLE)
		return dti->containees[index].al.type_info;

	if(dti->type == STRING || dti->type == BLOB)
		return UINT_NON_NULLABLE[1]; // this must be the containee here, so why not return the default

	// else it has to be an array
	return dti->containee;
}

static inline data_type_info* get_data_type_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return NULL;

	// same thing, if the index is out of bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return NULL;

	// index is now surely within bounds

	return get_data_type_info_for_containee_of_container_CONTAINITY_UNSAFE(dti, data, index);
}

static inline int get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* cached_return)
{
	// if a prior valid value was returned then no need to do anything further
	if(!IS_INVALID_DATA_POSITIONAL_INFO(cached_return))
		return 1;

	// for a tuple return a precomputed value
	if(dti->type == TUPLE)
	{
		(*cached_return) = dti->containees[index].al;
		return 1;
	}

	// case statement for strings, blobs and arrays

	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);
	uint32_t first_element_offset = prefix_bitmap_offset + get_prefix_bitmap_size_for_container_type_info(dti, data);

	data_type_info* containee_type_info = get_data_type_info_for_containee_of_container_CONTAINITY_UNSAFE(dti, data, index);

	if(containee_type_info->type == BIT_FIELD)
	{
		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
		{
			(*cached_return) = (data_positional_info){
				.bit_offset_in_prefix_bitmap = ((1 + containee_type_info->bit_field_size) * index) + 1,
				.bit_offset_to_is_valid_bit = ((1 + containee_type_info->bit_field_size) * index),
				.type_info = containee_type_info,
			};
			return 1;
		}
		else
		{
			(*cached_return) = (data_positional_info){
				.bit_offset_in_prefix_bitmap = containee_type_info->bit_field_size * index,
				.bit_offset_to_is_valid_bit = 0, // will be unused
				.type_info = containee_type_info,
			};
			return 1;
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		(*cached_return) = (data_positional_info){
			.byte_offset = first_element_offset + containee_type_info->size * index,
			.bit_offset_to_is_valid_bit = index * needs_is_valid_bit_in_prefix_bitmap(containee_type_info), // gets set to 0, if it won't need a is_valid bit in prefix_bitmap
			.type_info = containee_type_info,
		};
		return 1;
	}
	else
	{
		(*cached_return) = (data_positional_info){
			.byte_offset_to_byte_offset = first_element_offset + get_value_size_on_page(dti->max_size) * index,
			.type_info = containee_type_info,
		};
		return 1;
	}
}

static inline int get_data_positional_info_for_containee_of_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* cached_return)
{
	// this is not a valid function call for a non container type
	if(!is_container_type_info(dti))
		return 0;

	// same thing, if the index is out of bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, cached_return);
}

static inline int is_containee_null_in_container_CONTAINITY_UNSAFE(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	// a non-nullable element can never be null
	if(!is_nullable_type_info(containee_pos_info->type_info))
		return 0;

	if(!is_variable_sized_type_info(containee_pos_info->type_info))
	{
		// must have a bit in is_valid bitmap
		return get_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info->bit_offset_to_is_valid_bit) == 0;
	}
	else
	{
		// variable size element is NULL if the byte_offset in the tuple of the containee is 0
		return read_value_from_page(data + containee_pos_info->byte_offset_to_byte_offset, dti->max_size) == 0;
	}
}

static inline int is_containee_null_in_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 1;

	// make sure that index is withint bounds, else it is said to be NULL
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 1;

	return is_containee_null_in_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
}

static inline const void* get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// if it is already null return NULL
	if(is_containee_null_in_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info))
		return NULL;

	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	if(containee_pos_info->type_info->type == BIT_FIELD)
		return data + get_offset_to_prefix_bitmap_for_container_type_info(dti); // returning the pointer to the completee bitmap if the element is a bitfield
	else if(!is_variable_sized_type_info(containee_pos_info->type_info))
		return data + containee_pos_info->byte_offset;
	else
		return data + read_value_from_page(data + containee_pos_info->byte_offset_to_byte_offset, dti->max_size);
}

static inline const void* get_pointer_to_containee_from_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return NULL;

	// make sure that index is withint bounds, else it is said to be NULL
	if(index >= get_element_count_for_container_type_info(dti, data))
		return NULL;

	return get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
}

static inline uint32_t get_size_of_containee_from_container_CONTAINITY_UNSAFE(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	if(containee_pos_info->type_info->type == BIT_FIELD)
		return 0;
	else if(!is_variable_sized_type_info(containee_pos_info->type_info))
		return dti->size;
	else
	{
		const void* containee = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
		if(containee == NULL) // a NULL varibale sized element is said to be not containing any space
			return 0;
		else
			return get_size_for_type_info(containee_pos_info->type_info, containee);
	}
}

static inline uint32_t get_size_of_containee_from_container(const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// must be a container type info to call this function
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is withint bounds
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return get_size_of_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
}

static inline int get_user_value_for_type_info(user_value* uval, const data_type_info* dti, const void* data)
{
	switch(dti->type)
	{
		case BIT_FIELD :
		{
			return 0;
		}
		case UINT :
		{
			uval->is_NULL = 0;
			uval->uint_value = deserialize_uint64(data, dti->size);
			break;
		}
		case INT :
		{
			uval->is_NULL = 0;
			uval->int_value = deserialize_int64(data, dti->size);
			break;
		}
		case FLOAT :
		{
			uval->is_NULL = 0;
			if(dti->size == sizeof(float))
				uval->float_value = deserialize_float(data);
			else if(dti->size == sizeof(double))
				uval->double_value = deserialize_double(data);
			else if(dti->size == sizeof(long double))
				uval->long_double_value = deserialize_long_double(data);
			break;
		}
		case LARGE_UINT :
		{
			uval->is_NULL = 0;
			uval->large_uint_value = deserialize_uint256(data, dti->size);
			break;
		}
		case STRING :
		{
			uval->is_NULL = 0;
			// grab pointer to the first byte, and the element_count of the string, since it is inherently an array of non-nullable fixed length elements, they are placed sequential after the first byte
			// and the first byte is right where the prefix ends, there is not prefix_bitmap, so we can use its offset as index to first possible byte
			uval->string_value = data + get_offset_to_prefix_bitmap_for_container_type_info(dti);
			uval->string_size = get_element_count_for_container_type_info(dti, data);
			// this string could be null terminated
			uval->string_size = strnlen(uval->string_value, uval->string_size);
			break;
		}
		case BLOB :
		{
			uval->is_NULL = 0;
			// grab pointer to the first byte, and the element_count of the blob, since it is inherently an array of non-nullable fixed length elements, they are placed sequential after the first byte
			// and the first byte is right where the prefix ends, there is not prefix_bitmap, so we can use its offset as index to first possible byte
			uval->blob_value = data + get_offset_to_prefix_bitmap_for_container_type_info(dti);
			uval->blob_size = get_element_count_for_container_type_info(dti, data);
			break;
		}
		case TUPLE :
		{
			uval->is_NULL = 0;
			uval->tuple_value = data;
			break;
		}
		case ARRAY :
		{
			uval->is_NULL = 0;
			uval->array_value = data;
			break;
		}
	}

	return 1;
}

static inline int get_user_value_to_containee_from_container_CONTAINITY_UNSAFE(user_value* uval, const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);
	const void* containee = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);

	// if it is null return NULL_USER_VALUE
	if(containee == NULL)
	{
		uval->is_NULL = 1;
		return 1;
	}

	switch(containee_pos_info->type_info->type)
	{
		case BIT_FIELD :
		{
			uval->is_NULL = 0;
			uval->bit_field_value = get_bits(containee, containee_pos_info->bit_offset_in_prefix_bitmap, containee_pos_info->bit_offset_in_prefix_bitmap + containee_pos_info->type_info->bit_field_size - 1);
			return 1;
		}
		default :
			return get_user_value_for_type_info(uval, containee_pos_info->type_info, containee);
	}
}

static inline int get_user_value_to_containee_from_container(user_value* uval, const data_type_info* dti, const void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// dti has to be a container type, else we can not index it and so we return OUT_OF_BOUNDS_USER_VALUE
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is within bounds, else it is said to be OUT_OF_BOUNDS_USER_VALUE
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return get_user_value_to_containee_from_container_CONTAINITY_UNSAFE(uval, dti, data, index, containee_pos_info);
}

static inline int move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	// if this element is not variable sized then fail
	if(!is_variable_sized_type_info(containee_pos_info->type_info))
		return 0;

	void* containee = (void*) get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
	if(containee == NULL) // a null conatinee can not be sent to the end of the container
		return 0;

	uint32_t containee_byte_offset = containee - data;
	uint32_t containee_size = get_size_for_type_info(containee_pos_info->type_info, containee);
	uint32_t container_size = get_size_for_type_info(dti, data);

	// the containee is at the end of the container, if this check passes, if so, no need to do anything
	if(containee_byte_offset + containee_size == container_size)
		return 0;

	// perform left rotation to psuh the containee at index to the end of the container
	memory_left_rotate(containee, container_size - containee_byte_offset, containee_size);

	for(uint32_t i = 0; i < get_element_count_for_container_type_info(dti, data); i++)
	{
		data_positional_info pos_info_i = INVALID_DATA_POSITIONAL_INFO;
		get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, i, &pos_info_i);

		// the offsets have to be adjusted but not for the index-th element and not for the fixed sized elements
		if(i == index || !is_variable_sized_type_info(pos_info_i.type_info))
			continue;

		// move all offsets to elements that came after the containee_byte_offset front by the containee_size
		uint32_t byte_offset_i = read_value_from_page(data + pos_info_i.byte_offset_to_byte_offset, dti->max_size);

		if(byte_offset_i > containee_byte_offset)
			write_value_to_page(data + pos_info_i.byte_offset_to_byte_offset, dti->max_size, byte_offset_i - containee_size);
	}

	// finally update the offset of the index-th element
	write_value_to_page(data + containee_pos_info->byte_offset_to_byte_offset, dti->max_size, container_size - containee_size);
	return 1;
}

static inline int move_variable_sized_containee_to_end_of_container(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is withint bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
}

static inline uint32_t initialize_minimal_data_for_type_info(const data_type_info* dti, void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
	{
		memory_set(data, 0, dti->size);
		return dti->size;
	}
	else
	{
		memory_set(data, 0, dti->min_size);
		// if it has size set it to min_size, element_count if exists on the data is set to 0 by the above statement
		if(has_size_in_its_prefix_for_container_type_info(dti))
			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, dti->min_size);
		return dti->min_size;
	}
}

static inline int are_zeroes(const char* data, uint32_t data_size)
{
	for(uint32_t i = 0; i < data_size; i++)
		if(data[i] != 0)
			return 0;
	return 1;
}

static inline int is_minimal_data_for_type_info(const data_type_info* dti, const void* data)
{
	if(dti->type == BIT_FIELD)
		return 0;

	if(!is_variable_sized_type_info(dti))
		return are_zeroes(data, dti->size); // if not variable sized the completed content must be 0
	else
	{
		uint32_t data_size = get_size_for_type_info(dti, data);
		if(data_size != dti->min_size) // if variable sized the size must equal min_size
			return 0;

		// and the remaining content must be all zeroes
		if(has_size_in_its_prefix_for_container_type_info(dti))
			return are_zeroes(data, get_offset_to_prefix_size_for_container_type_info(dti)) &&
					are_zeroes(data + get_offset_to_prefix_size_for_container_type_info(dti) + get_value_size_on_page(dti->max_size),
								data_size - (get_offset_to_prefix_size_for_container_type_info(dti) + get_value_size_on_page(dti->max_size)));
		else
			return are_zeroes(data, data_size);
	}
}

static inline int set_containee_to_NULL_in_container_CONTAINITY_UNSAFE(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// if it is already null, succeed
	if(is_containee_null_in_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info))
		return 1;

	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	// a non-nullable element can never be null
	if(!is_nullable_type_info(containee_pos_info->type_info))
		return 0;

	if(needs_is_valid_bit_in_prefix_bitmap(containee_pos_info->type_info)) // must be a BIT_FIELD or a fixed sized field
		reset_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info->bit_offset_to_is_valid_bit);
	else // else it is a variable sized field
	{
		uint32_t container_size = get_size_for_type_info(dti, data);
		uint32_t containee_size = get_size_for_type_info(containee_pos_info->type_info, get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info));

		// move containee to the end of the container
		move_variable_sized_containee_to_end_of_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);

		// set containee offset to 0
		write_value_to_page(data + containee_pos_info->byte_offset_to_byte_offset, dti->max_size, 0);

		// if it has size in its prefix deduct containee_size from it
		if(has_size_in_its_prefix_for_container_type_info(dti))
			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, container_size - containee_size);
	}

	return 1;
}

static inline int set_containee_to_NULL_in_container(const data_type_info* dti, void* data, uint32_t index, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return set_containee_to_NULL_in_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
}

static inline int can_set_user_value_for_type_info(const data_type_info* dti, const void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval)
{
	if(is_user_value_NULL(uval))
		return 0;

	if(dti->type == BIT_FIELD)
		return 0;

	// if it is fixed sized, then no need to check for max_size_increment
	if(!is_variable_sized_type_info(dti))
		return 1;

	switch(dti->type)
	{
		case STRING :
		{
			user_value uval_t = *uval;

			// limit the string length
			uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);

			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval_t.string_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case BLOB :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval->blob_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case TUPLE :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->tuple_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		case ARRAY :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->array_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;
			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

static inline int set_user_value_for_type_info(const data_type_info* dti, void* data, int is_valid, uint32_t max_size_increment_allowed, const user_value* uval)
{
	if(is_user_value_NULL(uval))
		return 0;

	if(dti->type == BIT_FIELD)
		return 0;

	// if it is fixed sized, then no need to check for max_size_increment
	if(!is_variable_sized_type_info(dti))
	{
		switch(dti->type)
		{
			case UINT :
			{
				serialize_uint64(data, dti->size, uval->uint_value);
				return 1;
			}
			case INT :
			{
				serialize_int64(data, dti->size, uval->int_value);
				return 1;
			}
			case FLOAT :
			{
				if(dti->size == sizeof(float))
					serialize_float(data, uval->float_value);
				else if(dti->size == sizeof(double))
					serialize_double(data, uval->double_value);
				else if(dti->size == sizeof(long double))
					serialize_long_double(data, uval->long_double_value);
				return 1;
			}
			case LARGE_UINT :
			{
				serialize_uint256(data, dti->size, uval->large_uint_value);
				return 1;
			}
			case STRING :
			{
				user_value uval_t = *uval;

				// limit the string length
				uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);
				uval_t.string_size = min(uval_t.string_size, dti->size);

				// copy contents to data
				memory_move(data, uval_t.string_value, uval_t.string_size);
				// padd remaining bytes to 0
				if(uval_t.string_size < dti->size)
					memory_set(data + uval_t.string_size, 0, dti->size - uval_t.string_size);
				return 1;
			}
			case BLOB :
			{
				user_value uval_t = *uval;

				uval_t.blob_size = min(uval_t.blob_size, dti->size);
				// copy contents to data
				memory_move(data, uval_t.blob_value, uval_t.blob_size);
				return 1;
			}
			case TUPLE :
			{
				// copy contents to data
				if(uval != EMPTY_USER_VALUE) // if user provided this pointer, then the tuple_value is NULL, hence we need to initialize a minimal tuple at this position
					memory_move(data, uval->tuple_value, dti->size);
				else
					initialize_minimal_data_for_type_info(dti, data);
				return 1;
			}
			case ARRAY :
			{
				// copy contents to data
				if(uval != EMPTY_USER_VALUE) // if user provided this pointer, then the array_value is NULL, hence we need to initialize a minimal tuple at this position
					memory_move(data, uval->array_value, dti->size);
				else
					initialize_minimal_data_for_type_info(dti, data);
				return 1;
			}
			default :
			{
				return 0;
			}
		}
	}

	// now we know for sure that it is a variable sized type
	// it can be only one of these composite types
	switch(dti->type)
	{
		case STRING :
		{
			user_value uval_t = *uval;

			// limit the string length
			uval_t.string_size = strnlen(uval_t.string_value, uval_t.string_size);

			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval_t.string_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// write element count and copy contents to data
			write_value_to_page(data, dti->max_size, uval_t.string_size);
			memory_move(data + get_value_size_on_page(dti->max_size), uval_t.string_value, uval_t.string_size);
			return 1;
		}
		case BLOB :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = get_value_size_on_page(dti->max_size) + uval->blob_size;

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// write element count and copy contents to data
			write_value_to_page(data, dti->max_size, uval->blob_size);
			memory_move(data + get_value_size_on_page(dti->max_size), uval->blob_value, uval->blob_size);
			return 1;
		}
		case TUPLE :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->tuple_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// copy contents to data
			if(uval != EMPTY_USER_VALUE)
				memory_move(data, uval->tuple_value, dti->size);
			else
				initialize_minimal_data_for_type_info(dti, data);
			return 1;
		}
		case ARRAY :
		{
			uint32_t old_size = is_valid ? get_size_for_type_info(dti, data) : 0;
			uint32_t new_size = (uval == EMPTY_USER_VALUE) ? dti->min_size : get_size_for_type_info(dti, uval->array_value);

			if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
				return 0;

			// copy contents to data
			if(uval != EMPTY_USER_VALUE)
				memory_move(data, uval->array_value, dti->size);
			else
				initialize_minimal_data_for_type_info(dti, data);
			return 1;
		}
		default :
		{
			return 0;
		}
	}
}

static inline int can_set_user_value_to_containee_in_container_CONTAINITY_UNSAFE(const data_type_info* dti, const void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval, data_positional_info* containee_pos_info)
{
	// fetch information about containee
	get_data_positional_info_for_containee_of_container_CONTAINITY_USAFE(dti, data, index, containee_pos_info);

	// if uval is NULL, set it to NULL
	// this will never increment the size requirement, hence no checks required
	if(is_user_value_NULL(uval))
		return is_nullable_type_info(containee_pos_info->type_info);

	// there will already be enough space for the fixed sized containee
	if(!is_variable_sized_type_info(containee_pos_info->type_info))
		return 1;

	uint32_t old_container_size = get_size_for_type_info(dti, data);
	max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - old_container_size);

	const void* containee = get_pointer_to_containee_from_container_CONTAINITY_UNSAFE(dti, data, index, containee_pos_info);
	int is_old_containee_offset_valid = (containee != NULL);
	if(containee == NULL)
		containee = data + old_container_size;

	return can_set_user_value_for_type_info(containee_pos_info->type_info, containee, is_old_containee_offset_valid, max_size_increment_allowed, uval);
}

static inline int can_set_user_value_to_containee_in_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	return can_set_user_value_to_containee_in_container_CONTAINITY_UNSAFE(dti, data, index, max_size_increment_allowed, uval, containee_pos_info);
}

static inline int set_user_value_to_containee_in_container(const data_type_info* dti, void* data, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval, data_positional_info* containee_pos_info)
{
	// dti has to be a container type
	if(!is_container_type_info(dti))
		return 0;

	// make sure that index is within bounds, else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// if uval is NULL, set it to NULL
	// this will never increment the size requirement, hence no checks required
	if(is_user_value_NULL(uval))
		return set_containee_to_NULL_in_container(dti, data, index, containee_pos_info);

	// now we are sure that uval is not NULL

	// fetch information about containee
	get_data_positional_info_for_containee_of_container(dti, data, index, containee_pos_info);

	if(!is_variable_sized_type_info(containee_pos_info->type_info))
	{
		// if it has is_valid_bit then set it
		if(needs_is_valid_bit_in_prefix_bitmap(containee_pos_info->type_info)) // must be a BIT_FIELD or a fixed sized field
			set_bit(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info->bit_offset_to_is_valid_bit);

		if(containee_pos_info->type_info->type == BIT_FIELD)
		{
			set_bits(data + get_offset_to_prefix_bitmap_for_container_type_info(dti), containee_pos_info->bit_offset_in_prefix_bitmap, containee_pos_info->bit_offset_in_prefix_bitmap + containee_pos_info->type_info->bit_field_size - 1, uval->bit_field_value);
			return 1;
		}
		else
		{
			void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index, containee_pos_info);
			return set_user_value_for_type_info(containee_pos_info->type_info, containee, 1 /* this attribute is NO-OP here */, max_size_increment_allowed, uval);
		}
	}
	else
	{
		// first thing we need to do is send the concerned containee to the end of the container
		move_variable_sized_containee_to_end_of_container(dti, data, index, containee_pos_info);

		uint32_t old_container_size = get_size_for_type_info(dti, data);
		max_size_increment_allowed = min(max_size_increment_allowed, dti->max_size - old_container_size);

		void* containee = (void*) get_pointer_to_containee_from_container(dti, data, index, containee_pos_info);
		uint32_t old_containee_size = 0;
		int is_old_containee_offset_valid = (containee != NULL);

		// if the containee is NULL, then old_containee_size = 0
		// and the position to insert the containee is at the end of the container
		if(containee == NULL)
			containee = data + old_container_size;
		else
			old_containee_size = get_size_for_type_info(containee_pos_info->type_info, containee);

		int result = set_user_value_for_type_info(containee_pos_info->type_info, containee, is_old_containee_offset_valid, max_size_increment_allowed, uval);

		// if result was a success, and the old_containee_offset was in-valid i.e 0, then set it
		if(result && !is_old_containee_offset_valid)
			write_value_to_page(data + containee_pos_info->byte_offset_to_byte_offset, dti->max_size, old_container_size);

		// if the container has size in prefix then update it
		if(result && has_size_in_its_prefix_for_container_type_info(dti))
		{
			uint32_t new_containee_size = get_size_for_type_info(containee_pos_info->type_info, containee);

			uint32_t new_container_size = old_container_size - old_containee_size + new_containee_size;

			write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_container_size);
		}

		return result;
	}
}

static inline int can_expand_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// it must be variable element count container
	if(!is_variable_element_count_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count], else fail
	if(index > get_element_count_for_container_type_info(dti, data))
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count + slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;
	}
	else
	{
		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		new_size = old_size + (byte_offset_size * slots);
	}

	if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
		return 0;

	return 1;
}

static inline int expand_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots, uint32_t max_size_increment_allowed)
{
	// it must be variable element count container
	if(!is_variable_element_count_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count], else fail
	if(index > get_element_count_for_container_type_info(dti, data))
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count + slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new bits
		for(uint32_t i = 0; i < old_element_count - index; i++)
		{
			uint32_t copy_from_bit = (old_element_count - 1 - i) * prefix_bits_necessary_for_1_containee;
			uint32_t copy_to_bit = (new_element_count - 1 - i) * prefix_bits_necessary_for_1_containee;
			uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit + prefix_bits_necessary_for_1_containee - 1);
			set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit + prefix_bits_necessary_for_1_containee - 1, t);
		}

		// make new slots 0
		for(uint32_t i = 0; i < slots; i++)
		{
			uint32_t to_zero_bit = (index + i) * prefix_bits_necessary_for_1_containee;
			set_bits(data + prefix_bitmap_offset, to_zero_bit, to_zero_bit + prefix_bits_necessary_for_1_containee - 1, 0);
		}

		// zero out unused bits in prefix bitmap
		{
			uint64_t total_bits_in_prefix_bitmap = new_element_count * prefix_bits_necessary_for_1_containee;
			uint64_t upper_limit = UINT_ALIGN_UP(total_bits_in_prefix_bitmap, 8);
			if(upper_limit > total_bits_in_prefix_bitmap)
				set_bits(data + prefix_bitmap_offset, total_bits_in_prefix_bitmap, upper_limit - 1, 0);
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_old_size = bitmap_size_in_bytes(old_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new bits if required
		if(prefix_bitmap_old_size != prefix_bitmap_new_size)
		{
			uint32_t copy_from = prefix_bitmap_offset + prefix_bitmap_old_size;
			uint32_t copy_to = prefix_bitmap_offset + prefix_bitmap_new_size;
			memory_move(data + copy_to, data + copy_from, old_size - copy_from);
		}

		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info)) // if these element have is_valid bits in prefix, then move them accordingly or 0 them if they are new
		{
			for(uint32_t i = 0; i < old_element_count - index; i++)
			{
				uint32_t copy_from_bit = old_element_count - 1 - i;
				uint32_t copy_to_bit = new_element_count - 1 - i;
				uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit);
				set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit, t);
			}

			// make new slots 0
			for(uint32_t i = 0; i < slots; i++)
			{
				uint32_t to_zero_bit = (index + i);
				reset_bit(data + prefix_bitmap_offset, to_zero_bit);
			}
		}

		// calculate the offset to the array of containees
		uint32_t offset_to_first_element = prefix_bitmap_offset + prefix_bitmap_new_size;

		// make room from new slots for the fixed sized containees
		memory_move(data + offset_to_first_element + ((index + slots) * byte_size), data + offset_to_first_element + (index * byte_size), (old_element_count - index) * byte_size);

		// zero out the new slots
		memory_set(data + offset_to_first_element + (index * byte_size), 0, slots * byte_size);

		// zero out unused bits in prefix bitmap
		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info))
		{
			uint64_t total_bits_in_prefix_bitmap = new_element_count /* * needs_is_valid_bit_in_prefix_bitmap(containee_type_info)*/ ; // multiplying with 1 is no fun
			uint64_t upper_limit = UINT_ALIGN_UP(total_bits_in_prefix_bitmap, 8);
			if(upper_limit > total_bits_in_prefix_bitmap)
				set_bits(data + prefix_bitmap_offset, total_bits_in_prefix_bitmap, upper_limit - 1, 0);
		}
	}
	else
	{
		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		// calculate new size and check for size increments
		new_size = old_size + (byte_offset_size * slots);
		if(new_size > dti->max_size || (new_size > old_size && new_size - old_size > max_size_increment_allowed))
			return 0;

		// make room for new slots
		uint32_t copy_from = prefix_bitmap_offset + byte_offset_size * index;
		uint32_t copy_to = prefix_bitmap_offset + byte_offset_size * (index + slots);
		memory_move(data + copy_to, data + copy_from, old_size - copy_from);

		// zero out the new slots
		memory_set(data + copy_from, 0, copy_to - copy_from);

		// since all the varibale length data is moved back by (copy_to - copy_from) bytes, we need to update their offsets
		for(uint32_t i = 0; i < new_element_count; i++)
		{
			uint32_t offset = read_value_from_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size);
			if(offset != 0)
				write_value_to_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size, offset + (copy_to - copy_from));
		}
	}

	// update the size and element count in the prefix
	if(has_size_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);
	if(has_element_count_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size, new_element_count);

	return 1;
}

static inline int can_discard_from_container(const data_type_info* dti, const void* data, uint32_t index, uint32_t slots)
{
	// it must be variable element count container
	if(!is_variable_element_count_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count-1], else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// must have atleast slots number of slots after first index number of slots
	if(get_element_count_for_container_type_info(dti, data) - index < slots)
		return 0;

	// else it will surely succeed
	return 1;
}

static inline int discard_from_container(const data_type_info* dti, void* data, uint32_t index, uint32_t slots)
{
	// it must be variable element count container
	if(!is_variable_element_count_container_type_info(dti))
		return 0;

	// make sure that index is within [0, element_count-1], else fail
	if(index >= get_element_count_for_container_type_info(dti, data))
		return 0;

	// must have atleast slots number of slots after first index number of slots
	if(get_element_count_for_container_type_info(dti, data) - index < slots)
		return 0;

	// default succeed 0 slots
	if(slots == 0)
		return 1;

	// since it is an array, string or a blob of variable element count
	// it's containee is bound to be fixed
	data_type_info* containee_type_info = dti->containee;

	// fetch the old element_count, and calculate the new_element_count
	uint32_t old_element_count = get_element_count_for_container_type_info(dti, data);
	uint32_t new_element_count = old_element_count - slots;

	// fetch the old_size, new_size will differ based on its containee
	uint32_t old_size = get_size_for_type_info(dti, data);
	uint32_t new_size = 0;

	// prefix size and prefix element count will remain in the same place
	// prefix_bitmap_offset will also remain in the same place
	uint32_t prefix_bitmap_offset = get_offset_to_prefix_bitmap_for_container_type_info(dti);

	if(containee_type_info->type == BIT_FIELD)
	{
		// all of the content of the containee is in its prefix_bitmap
		uint32_t prefix_bits_necessary_for_1_containee = needs_is_valid_bit_in_prefix_bitmap(containee_type_info) + containee_type_info->bit_field_size;

		// calculate new size
		new_size = prefix_bitmap_offset + bitmap_size_in_bytes(prefix_bits_necessary_for_1_containee * new_element_count);

		// move succeeding bits to prior location
		for(uint32_t i = 0; i < old_element_count - (index + slots); i++)
		{
			uint32_t copy_from_bit = ((index + slots) + i) * prefix_bits_necessary_for_1_containee;
			uint32_t copy_to_bit = (index + i) * prefix_bits_necessary_for_1_containee;
			uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit + prefix_bits_necessary_for_1_containee - 1);
			set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit + prefix_bits_necessary_for_1_containee - 1, t);
		}
	}
	else if(!is_variable_sized_type_info(containee_type_info))
	{
		uint32_t prefix_bitmap_old_size = bitmap_size_in_bytes(old_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));
		uint32_t prefix_bitmap_new_size = bitmap_size_in_bytes(new_element_count * needs_is_valid_bit_in_prefix_bitmap(containee_type_info));

		uint32_t byte_size = containee_type_info->size;

		uint32_t new_containees_size = byte_size * new_element_count;

		// calculate new size and check for size increments
		new_size = prefix_bitmap_offset + prefix_bitmap_new_size + new_containees_size;

		if(needs_is_valid_bit_in_prefix_bitmap(containee_type_info)) // if these element have is_valid bits in prefix, move them back
		{
			for(uint32_t i = 0; i < old_element_count - (index + slots); i++)
			{
				uint32_t copy_from_bit = ((index + slots) + i);
				uint32_t copy_to_bit = (index + i);
				uint64_t t = get_bits(data + prefix_bitmap_offset, copy_from_bit, copy_from_bit);
				set_bits(data + prefix_bitmap_offset, copy_to_bit, copy_to_bit, t);
			}
		}

		// move data containing containees, if the bitmap shrunk
		if(prefix_bitmap_old_size != prefix_bitmap_new_size)
		{
			uint32_t copy_from = prefix_bitmap_offset + prefix_bitmap_old_size;
			uint32_t copy_to = prefix_bitmap_offset + prefix_bitmap_new_size;
			memory_move(data + copy_to, data + copy_from, old_size - copy_from);
		}

		// calculate the offset to the array of containees
		uint32_t offset_to_first_element = prefix_bitmap_offset + prefix_bitmap_new_size;

		// discard the concerning slots
		memory_move(data + offset_to_first_element + (index * byte_size), data + offset_to_first_element + ((index + slots) * byte_size), (old_element_count - (index + slots)) * byte_size);
	}
	else
	{
		// first make the concerning slots NULL
		for(uint32_t i = 0; i < slots; i++)
		{
			data_positional_info pos_info_i = INVALID_DATA_POSITIONAL_INFO;
			set_containee_to_NULL_in_container(dti, data, index + i, &pos_info_i);
		}

		// update the old size that we have cached
		old_size = get_size_for_type_info(dti, data);

		uint32_t byte_offset_size = get_value_size_on_page(dti->max_size);

		// calculate new size
		new_size = old_size - (byte_offset_size * slots);

		// make room for new slots
		uint32_t copy_from = prefix_bitmap_offset + byte_offset_size * (index + slots);
		uint32_t copy_to = prefix_bitmap_offset + byte_offset_size * index;
		memory_move(data + copy_to, data + copy_from, old_size - copy_from);

		// since all the varibale length data is moved back by (copy_from - copy_to) bytes, we need to update their offsets
		for(uint32_t i = 0; i < new_element_count; i++)
		{
			uint32_t offset = read_value_from_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size);
			if(offset != 0)
				write_value_to_page(data + prefix_bitmap_offset + (i * byte_offset_size), dti->max_size, offset - (copy_from - copy_to));
		}
	}

	// update the size and element count in the prefix
	if(has_size_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_size_for_container_type_info(dti), dti->max_size, new_size);
	if(has_element_count_in_its_prefix_for_container_type_info(dti))
		write_value_to_page(data + get_offset_to_prefix_element_count_for_container_type_info(dti), dti->max_size, new_element_count);

	return 1;
}

#endif