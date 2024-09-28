#ifndef PAGE_LAYOUT_UTIL_H
#define PAGE_LAYOUT_UTIL_H

#include<stdint.h>
#include<inttypes.h>

// here we assume that you only store offsets on the page, hence the value size is calculated accordingly, (see source file for more information on implementation)
// like if your page_size is 256 then you only need to store offsets that are in range 0 - 255 all of which fits in 8 bits
// you will probably also store size/element count and alike metadata information, so your offsets and sizes will mostly always fit these value
// an offset of 256 will be outside the page, so if you need to store that then set it to 0, and them in return read it as page_size
// when your size is 256, then you must be storing size only for a variable length data, this data because it stores size in prefic will never be 0 bytes, hece use 0 for page_size of size
// This optimization is done to save off 1 byte worth of information of the offsets, sizes and element_counts, And computers generally tend to give us data pages that are almost always powers of 2.
uint32_t get_value_size_on_page(uint32_t page_size);

uint32_t read_value_from_page(const void* value, uint32_t page_size);

void write_value_to_page(void* value, uint32_t page_size, uint32_t to_write);

#endif