#ifndef PAGE_LAYOUT_UTIL_H
#define PAGE_LAYOUT_UTIL_H

#include<stdint.h>

uint32_t get_value_size_on_page(uint32_t page_size);

uint32_t read_value_from_page(void* value, uint32_t page_size);

#endif