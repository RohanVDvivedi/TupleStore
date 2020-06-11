#include<headers.h>

// signature before every SimpleTupleStorageModel database
char* signature = "<- Simple Tuple Storage Model ->";

void init_file_header(file_hdr* hdr, char* database_name, uint32_t page_size_in_bytes)
{
	memcpy(hdr->signature, signature, SIGNATURE_LENGTH);
	memcpy(hdr->database_name, database_name, DATABASE_NAME_LENGTH);

	hdr->tables_list_root_page_id = 0;
	hdr->columns_list_root_page_id = 0;
	hdr->indices_list_root_page_id = 0;
	hdr->columns_indices_list_root_page_id = 0;
	hdr->free_pages_list_root_page_id = 0;
	hdr->page_size_in_bytes = page_size_in_bytes;
	hdr->total_number_of_pages_used = 1;
}

void init_page_header(page_hdr* hdr, u8 page_type, page_layout layout)
{
	hdr->page_type = page_type;
	hdr->next_page_id = 0;
	hdr->overflow_page_id = 0;
	hdr->layout = layout;
}