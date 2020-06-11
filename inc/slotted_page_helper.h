#ifndef SLOTTED_PAGE_HELPER
#define SLOTTED_PAGE_HELPER

extern inline uint16_t get_number_of_tuples_stored(page_context* pg_cntxt)
{
	if(pg_cntxt->tuples_stored != 0)
	{
		return pg_cntxt->tuples_stored;
	}

	void* page_content = get_page_content(pg_cntxt);

	// the first 16 bits represent the number of tuples currently stored in the page

	pg_cntxt->tuples_stored = *((uint16_t*)(page_content));
	return pg_cntxt->tuples_stored;
}

extern inline uint16_t* get_tuple_offsets(page_context* pg_cntxt)
{
	void* page_content = get_page_content(pg_cntxt);

	// the first 16 bits represent the number of tuples currently stored in the page
	// hence the + 2
	// and then follows the array of 16 bit integers stating the offsets within page for each tuple that is stored

	return (uint16_t*)(page_content + 2);
}

// you must provide a tuple_no < tuples_stored for the page
// tuple offset being 0 mean, that there does not exist a tuple at that location
extern inline int is_tuple_deleted(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint16_t* tuple_offsets = get_tuple_offsets(pg_cntxt);
	return tuple_offsets[tuple_no] == 0;
}

// you must provide a tuple_no < tuples_stored for the page
// tuple offset being 0 mean, that there does not exist a tuple at that location
extern inline void mark_tuple_deleted(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint16_t* tuple_offsets = get_tuple_offsets(pg_cntxt);
	tuple_offsets[tuple_no] = 0;
}

#endif