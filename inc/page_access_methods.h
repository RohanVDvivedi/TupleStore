#ifndef PAGE_ACCESS_METHODS_H
#define PAGE_ACCESS_METHODS_H

typedef struct page_access_methods page_access_methods;
struct page_access_methods
{
	// locks a page for read or write, if successfull must return pointer to the in-memory of the page
	void* (*lock_page_read(void* context, uint32_t page_id));
	void* (*lock_page_write(void* context, uint32_t page_id));

	// releases lock on the page, accordingly
	int (*release_read_lock(void* context, void* pg_ptr));
	int (*release_write_lock(void* context, void* pg_ptr));

	// context to pass on every page access
	const void* context;
};

#endif