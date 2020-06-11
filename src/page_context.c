#include<page_context.h>

#include<layout_helper.h>
#include<tuple_array_helper.h>
#include<slotted_page_helper.h>

void init_page_context(page_context* pg_cntxt, uint32_t page_id, void* page, tuple_def* tuple_definition, const data_access_methods* dam)
{
	pg_cntxt->page_id = page_id;
	pg_cntxt->header = page;
	pg_cntxt->tuple_definition = tuple_definition;

	pg_cntxt->tuples_stored = 0;
	pg_cntxt->tuple_storage_limit = 0;

	pg_cntxt->dam = dam;
}

void* get_tuple(page_context* pg_cntxt, uint16_t tuple_no)
{
	page_hdr* pg_hdr = pg_cntxt->header;

	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			byte_size tuple_size = pg_cntxt->tuple_definition->size; // size of tuple in bytes
			tuple_storage_limit(pg_cntxt);
			if(tuple_no < pg_cntxt->tuple_storage_limit)
			{
				void* tuples = get_tuples(pg_cntxt);
				return tuples + (tuple_no * tuple_size);
			}
			else
				return 0;
		}
		case SLOTTED_PAGE :
		{
			get_number_of_tuples_stored(pg_cntxt);
			if(tuple_no < pg_cntxt->tuples_stored)
			{
				return get_page(pg_cntxt); + (get_tuple_offsets(pg_cntxt))[tuple_no];
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
			byte_size tuple_size = pg_cntxt->tuple_definition->size; 	// size of tuple in bytes
	
			void* tuples = get_tuples(pg_cntxt);

			uint32_t total_tuples_size = get_total_page_content_size(pg_cntxt);

			uint32_t occupied_tuples_size = tuple_size * pg_hdr->tuple_count_in_page;

			uint32_t vacant_bytes_size = total_tuples_size - occupied_tuples_size;

			uint16_t vacant_tuples_count = vacant_bytes_size / tuple_size;

			if(vacant_tuples_count < num_tuples_to_insert)
				num_tuples_to_insert = vacant_tuples_count;

			if(num_tuples_to_insert > 0)
			{
				memcpy(tuples + (pg_hdr->tuple_count_in_page * tuple_size),
						tuples_to_insert, num_tuples_to_insert * tuple_size);
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