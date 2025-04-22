#ifndef PAGE_LAYOUT_UTIL_H
#define PAGE_LAYOUT_UTIL_H

#include<stdint.h>
#include<inttypes.h>

// here we assume that you only store offsets on the page, hence the value size is calculated accordingly, (see source file for more information on implementation)
// like if your page_size is 256 then you only need to store offsets that are in range 0 - 255 all of which fit in 8 bits
// you will probably also store size/element count and alike metadata information, so your offsets and sizes will mostly always fit these value
// an offset of 256 will be outside the page, so if you need to store that then set it to 0, and then while reading it, return it as page_size in a uint32_t
// when your size is 256, then you must be storing size only for a variable length data, this data because it stores size in prefix will never be 0 bytes, hence use 0 for page_size value of any sizes that you may encounter
// This optimization is done to save off 1 byte worth of information of the offsets, sizes and element_counts, and computers generally tend to give us data pages that are almost always powers of 2.
static inline uint32_t get_value_size_on_page(uint32_t page_size)
{
	/*if(page_size <= (UINT32_C(1) << 8))
		return 1;
	else if(page_size <= (UINT32_C(1) << 16))
		return 2;
	else if(page_size <= (UINT32_C(1) << 24))
		return 3;
	else
		return 4;*/
	// This function is very frequently called inside the TupleStore, hence an attempt to make it jump free
	// using the line below
	return 4 - (page_size <= (UINT32_C(1) << 24)) - (page_size <= (UINT32_C(1) << 16)) - (page_size <= (UINT32_C(1) << 8));
}

#include<serint/serial_int.h>

static inline uint32_t read_value_from_page(const void* value, uint32_t page_size)
{
	return deserialize_uint32(value, get_value_size_on_page(page_size));
}

static inline void write_value_to_page(void* value, uint32_t page_size, uint32_t to_write)
{
	serialize_uint32(value, get_value_size_on_page(page_size), to_write);
}

static inline void swap_values_on_page(void* value1, void* value2, uint32_t page_size)
{
	//memory_swap(value1, value2, get_value_size_on_page(page_size));

	// opting for an inline implementation over memory_swap function call

	char* v1 = value1;
	char* v2 = value2;
	uint32_t bytes_to_swap = get_value_size_on_page(page_size);
	while(bytes_to_swap > 0)
	{
		const char temp = (*v1);
		(*v1) = (*v2);
		(*v2) = temp;

		v1++;
		v2++;
		bytes_to_swap--;
	}
}

#endif