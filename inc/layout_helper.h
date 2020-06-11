#ifndef LAYOUT_HELPER_H
#define LAYOUT_HELPER_H

inline void* get_page(page_context* pg_cntxt)
{
	return pg_cntxt->header;
}

inline void* get_page_content(page_context* pg_cntxt)
{
	void* page = get_page(pg_cntxt);
	return page + sizeof(page_hdr);
}

inline uint16_t get_page_content_size(page_context* pg_cntxt)
{
	return pg_cntxt->dam->page_size_in_bytes - sizeof(page_hdr);
}

#endif