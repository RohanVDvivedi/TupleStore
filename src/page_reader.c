#include<page_reader.h>

void* get_tuple_pointer(const data_access_methods* dam, void* page, uint16_t tuple_no)
{
	page_hdr* phdr = (page_hdr*) page;
	void* tuples = page + sizeof(page_hdr);

	if(tuple_no < phdr->tuple_count_in_page)
		return tuples + (tuple_no * phdr->tuple_size_in_bytes);
	else
		return 0;
}

uint16_t append_tuples(const data_access_methods* dam, void* page, void* tuples_to_insert, uint16_t num_tuples_to_insert)
{
	page_hdr* phdr = (page_hdr*) page;
	void* tuples = page + sizeof(page_hdr);

	uint32_t vacant_bytes_size = dam->page_size_in_bytes - sizeof(page_hdr) - (phdr->tuple_size_in_bytes * phdr->tuple_count_in_page);
	uint16_t vacant_tuples_size = vacant_bytes_size / phdr->tuple_size_in_bytes;

	if(vacant_tuples_size < num_tuples_to_insert)
		num_tuples_to_insert = vacant_tuples_size;

	if(tuples_to_insert > 0)
	{
		memcpy(tuples + (phdr->tuple_count_in_page * phdr->tuple_size_in_bytes),
				tuples_to_insert, num_tuples_to_insert * phdr->tuple_size_in_bytes);
		phdr->tuple_count_in_page += num_tuples_to_insert;
	}

	return num_tuples_to_insert;
}