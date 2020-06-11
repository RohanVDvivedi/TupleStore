#ifndef TUPLE_ARRAY_HELPER
#define TUPLE_ARRAY_HELPER

// maximum number of tuples that you can store in a tuple array storage layout
uint16_t tuple_storage_limit(page_context* pg_cntxt)
{
	if(pg_cntxt->tuple_storage_limit != 0)
	{
		return pg_cntxt->tuple_storage_limit;
	}

	uint16_t total_page_content_size = get_total_page_content_size(pg_cntxt);

	/*
	**	total bits for page content = 8 * total_page_content_size
	**	bits required to store a tuple = 8 * page_hdr->tuple_definition->size + 1
	**	extra 1 lets us store if the tuple is deleted or not
	**  tuples_limit = total bits for page content / bits required to store a tuple
	*/

	pg_cntxt->tuple_storage_limit = (8 * total_page_content_size) / (8 * page_hdr->tuple_definition->size + 1);

	return pg_cntxt->tuple_storage_limit;
}

// array of bytes, where each bit is used to mark if a tuple is deleted or not
uint8_t* tuple_deleted_bits(page_context* pg_cntxt)
{
	void* page_content = get_page_content(pg_cntxt);
	// tuple deleted bits are the first thing stored in the tuple array layout in the 
	return (uint8_t*)(page_content);
}

// size of array of bytes, where each bit is used to mark if a tuple is deleted or not
uint16_t tuple_deleted_bits_size_in_bytes(page_context* pg_cntxt)
{
	return (pg_cntxt->tuple_storage_limit / 8) + (((pg_cntxt->tuple_storage_limit % 8) > 0) ? 1 : 0);
}

void* get_tuples(page_context* pg_cntxt)
{
	void* page_content = get_page_content(pg_cntxt);

	// tuples are stored after the deleted bits array 

	return page_content + tuple_deleted_bits_size_in_bytes(pg_cntxt);
}

// you must provide a tuple_no < tuple_storage_limit for the page
int is_tuple_deleted(page_context* pg_cntxt, uint16_t tuple_no)
{
	uint8_t* deleted_bits = tuple_deleted_bits(pg_cntxt);
	return deleted_bits[tuples_no / 8] & (1 << (tuples_no % 8));
}

#endif