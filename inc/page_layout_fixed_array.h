#ifndef PAGE_LAYOUT_FIXED_ARRAY_H
#define PAGE_LAYOUT_FIXED_ARRAY_H

uint32_t get_minimum_page_size_for_fixed_array_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

#endif