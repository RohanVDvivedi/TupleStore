#ifndef TUPLE_ARRAY_HELPER
#define TUPLE_ARRAY_HELPER

// maximum number of tuples that you can store in a tuple array storage layout
extern inline void set_tuple_storage_limit(page_context* pg_cntxt)
{
	uint16_t page_content_size = get_page_content_size(pg_cntxt);

	/*
	**	total bits for page content = 8 * page_content_size
	**	bits required to store a tuple = (8 * page_hdr->tuple_definition->size) + 1
	**	extra 1 lets us store if the tuple exists at that index or not
	**  tuples_limit = total bits for page content / bits required to store a tuple
	*/

	pg_cntxt->tuple_storage_limit = (8 * page_content_size) / (8 * pg_cntxt->tuple_definition->size + 1);
}

// array of bytes, where each bit is used to mark if a tuple exists at that position or not
extern inline uint8_t* tuple_exists_bits(page_context* pg_cntxt)
{
	void* page_content = get_page_content(pg_cntxt);
	// tuple deleted bits are the first thing stored in the tuple array layout in the 
	return (uint8_t*)(page_content);
}

// size of array of bytes, where each bit is used to mark if a tuple exists at that position or not
extern inline uint16_t tuple_exists_bits_size_in_bytes(page_context* pg_cntxt)
{
	return (pg_cntxt->tuple_storage_limit / 8) + (((pg_cntxt->tuple_storage_limit % 8) > 0) ? 1 : 0);
}

extern inline void* get_tuples(page_context* pg_cntxt)
{
	void* page_content = get_page_content(pg_cntxt);

	// tuples are stored after the deleted bits array 

	return page_content + tuple_exists_bits_size_in_bytes(pg_cntxt);
}

// checks if tuple exists at a particular tuple_no in the page
extern inline int does_tuple_exist(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint8_t* exist_bits = tuple_exists_bits(pg_cntxt);
	return (exist_bits[tuple_no/8]&(1<<(tuple_no%8)));
}

extern inline void unset_tuple_exist(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint8_t* exist_bits = tuple_exists_bits(pg_cntxt);
	exist_bits[tuple_no/8] &= (~(1<<(tuple_no%8)));
}

extern inline void set_tuple_exist(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint8_t* exist_bits = tuple_exists_bits(pg_cntxt);
	exist_bits[tuple_no/8] |= (1<<(tuple_no%8));
}

#endif