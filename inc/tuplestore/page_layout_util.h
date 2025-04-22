#ifndef PAGE_LAYOUT_UTIL_H
#define PAGE_LAYOUT_UTIL_H

#include<stdint.h>
#include<inttypes.h>

/*
** Here we assume that you only store offsets on the page, hence the value size is calculated accordingly, (THERE IS NO OFF-BY-ONE ERROR, HERE)
**
** Lets say, if your page_size is 256 then you only need to store offsets that are in range 0 - 255 all of which fit in 8 bits
**
** page header is at offset 0 and it along with its size in the prefix (page_header_size excludes itself) will always occupy non-zero bytes
** hence offsets on the page can never be 0
** This allows 0 to be used for NULLs in tuple offsets (and other offsets) and 0 is used as PAGE_SIZE for the end_of_free_space pointer
**
** for tuples that are on the page, the non-zero total bytes of page_header, precludes any tuple or its attributes to ever be PAGE_SIZE number of bytes in size
** for offsets of variable size elements, we will just set them to 0, to represent NULLs
**
** This optimization (as explained above) is done to shave off 1 byte worth of information from sizes and element_counts on pages of powers of 256.
*/

// core functions that calculates the value of sizes, offsets and indices of elements on the page or inside tuples
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