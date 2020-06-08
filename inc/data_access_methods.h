#ifndef DATA_ACCESS_METHODS_H
#define DATA_ACCESS_METHODS_H

#include<stdint.h>

/*
**	This structure defines functions that provide access methods to the storage model to access database
*/

typedef struct data_access_methods data_access_methods;
struct data_access_methods
{
	void* (*open_data_file)(void* context);

	// locks a page for read or write, if successfull must return pointer to the in-memory of the page
	void* (*lock_page_read)(void* context, uint32_t page_id);
	void* (*lock_page_write)(void* context, uint32_t page_id);

	// releases lock on the page, accordingly
	int (*release_read_lock)(void* context, void* pg_ptr);
	int (*release_write_lock)(void* context, void* pg_ptr);

	int (*force_write_to_disk)(void* context, uint32_t page_id);

	int (*force_write_file_to_disk)(void* context);

	void* (*close_data_file)(void* context);

	// context to pass on every page access
	const void* context;
};

#endif