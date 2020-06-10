#include<page_context.h>

void* get_page(page_context* pg_cntxt)
{
	return pg_cntxt->header;
}

void* get_tuple(page_context* pg_cntxt, uint16_t tuple_no)
{
	page_hdr* pg_hdr = pg_cntxt->header;
	void* page = get_page(pg_cntxt);
	//const data_access_methods* dam = pg_cntxt->dam;

	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			if(tuple_no < pg_hdr->tuple_count_in_page)
			{
				void* tuples = page + sizeof(page_hdr);
				return tuples + (tuple_no * pg_hdr->tuple_size_in_bytes);
			}
			else
				return 0;
		}
		case SLOTTED_PAGE :
		{
			if(tuple_no < pg_hdr->tuple_count_in_page)
			{
				void* tuples_base_offset = page + sizeof(page_hdr);
				return tuples_base_offset + pg_hdr->tuple_offsets[tuple_no];
			}
			else
				return 0;
		}
		default :
		{
			return 0;
		}
	}
}

uint16_t append_tuples(page_context* pg_cntxt, void* tuples_to_insert, uint16_t num_tuples_to_insert)
{
	page_hdr* pg_hdr = pg_cntxt->header;
	void* page = get_page(pg_cntxt);
	const data_access_methods* dam = pg_cntxt->dam;

	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			void* tuples = page + sizeof(page_hdr);

			uint32_t total_tuples_size = dam->page_size_in_bytes - sizeof(page_hdr);

			uint32_t occupied_tuples_size = pg_hdr->tuple_size_in_bytes * pg_hdr->tuple_count_in_page;

			uint32_t vacant_bytes_size = total_tuples_size - occupied_tuples_size;

			uint16_t vacant_tuples_count = vacant_bytes_size / pg_hdr->tuple_size_in_bytes;

			if(vacant_tuples_count < num_tuples_to_insert)
				num_tuples_to_insert = vacant_tuples_count;

			if(num_tuples_to_insert > 0)
			{
				memcpy(tuples + (pg_hdr->tuple_count_in_page * pg_hdr->tuple_size_in_bytes),
						tuples_to_insert, num_tuples_to_insert * pg_hdr->tuple_size_in_bytes);
				pg_hdr->tuple_count_in_page += num_tuples_to_insert;
			}

			return num_tuples_to_insert;
		}
		case SLOTTED_PAGE :
		{
			return 0;
		}
		default :
		{
			return 0;
		}
	}
}