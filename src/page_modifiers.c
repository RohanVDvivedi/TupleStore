#include<page_modifiers.h>

int create_or_open_database(const data_access_methods* dam, char* database_name)
{
	// we can not use page_sizes lesser than the size of the heeader of files or pages
	// also page size must be multiple of 512
	if((dam->page_size_in_bytes < sizeof(file_hdr))
		|| (dam->page_size_in_bytes < sizeof(page_hdr))
		|| (dam->page_size_in_bytes % 512 != 0))
	{
		return 0;
	}

	dam->open_data_file(dam->context);

	void* file_hdr_page = dam->acquire_write_lock(dam->context, 0);

		file_hdr* fhdr = (file_hdr*)file_hdr_page;

		// existence of signature, means there is already a dayabase file setup
		if(strncmp(fhdr->signature, signature, SIGNATURE_LENGTH))
		{
			// initialize database file
			intialize_file_header(fhdr, database_name, dam->page_size_in_bytes);
		}
		// do not open to modify or re write anyother database
		else if(strncmp(fhdr->database_name, database_name, DATABASE_NAME_LENGTH))
		{
			dam->release_write_lock(dam->context, file_hdr_page);

			dam->close_data_file(dam->context);

			return 0;
		}

	dam->release_write_lock(dam->context, file_hdr_page);

	return 1;
}

void create_table(const data_access_methods* dam, char* table_name, row_def* columns);

void drop_table(const data_access_methods* dam, char* table_name);

int close_database(const data_access_methods* dam)
{
	return dam->close_data_file(dam->context);
}