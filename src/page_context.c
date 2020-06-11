#include<page_context.h>

#include<layout_helper.h>
#include<tuple_array_helper.h>
#include<slotted_page_helper.h>

void init_page_context(page_context* pg_cntxt, uint32_t page_id, void* page, tuple_def* tuple_definition, const data_access_methods* dam, int reset)
{
	pg_cntxt->page_id = page_id;
	pg_cntxt->header = page;
	pg_cntxt->tuple_definition = tuple_definition;

	pg_cntxt->tuples_stored = 0;
	pg_cntxt->tuple_storage_limit = 0;

	pg_cntxt->dam = dam;

	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			set_tuple_storage_limit(pg_cntxt);
			break;
		}
		case SLOTTED_PAGE :
		{
			set_number_of_tuples_stored(pg_cntxt);
			break;
		}
		default :
		{
			break;
		}
	}

	if(reset)
	{
		memset(get_page_content(pg_cntxt), 0, get_page_content_size(pg_cntxt));
	}
}

void* get_tuple(page_context* pg_cntxt, uint16_t tuple_no)
{
	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			if(tuple_no < pg_cntxt->tuple_storage_limit && does_tuple_exist(pg_cntxt, tuple_no))
			{
				void* tuples = get_tuples(pg_cntxt);
				return tuples + (tuple_no * pg_cntxt->tuple_definition->size);
			}
			else
				return 0;
		}
		case SLOTTED_PAGE :
		{
			if(tuple_no < pg_cntxt->tuples_stored && !is_tuple_deleted(pg_cntxt, tuple_no))
				return get_page(pg_cntxt) + get_tuple_offsets(pg_cntxt)[tuple_no];
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
	switch(pg_cntxt->header->layout)
	{
		case TUPLE_ARRAY :
		{
			byte_size tuple_size = pg_cntxt->tuple_definition->size; 	// size of tuple in bytes
	
			void* tuples = get_tuples(pg_cntxt);

			uint16_t tuples_next = 0;

			for(u2 i = 0; i < pg_cntxt->tuple_storage_limit; i++)
			{
				if(does_tuple_exist(pg_cntxt, i))
				{
					tuples_next = i + 1;
				}
			}

			if(tuples_next == pg_cntxt->tuple_storage_limit)
				return 0;

			uint16_t vacant_tuples_count = pg_cntxt->tuple_storage_limit - tuples_next;

			if(vacant_tuples_count < num_tuples_to_insert)
				num_tuples_to_insert = vacant_tuples_count;

			if(num_tuples_to_insert > 0)
			{
				memcpy(tuples + (tuples_next * tuple_size),
						tuples_to_insert, num_tuples_to_insert * tuple_size);
				for(u2 i = 0; i < num_tuples_to_insert; i++)
					set_tuple_exist(pg_cntxt, tuples_next + i);
			}

			return num_tuples_to_insert;
		}
		case SLOTTED_PAGE :
		{
			uint16_t new_tuple_offset = pg_cntxt->dam->page_size_in_bytes - pg_cntxt->tuple_definition->size;
			uint16_t* tuple_offsets = get_tuple_offsets(pg_cntxt);
			while(num_tuples_to_insert > 0)
			{
				if(pg_cntxt->tuples_stored > 0)
					new_tuple_offset = tuple_offsets[pg_cntxt->tuples_stored - 1] - pg_cntxt->tuple_definition->size;

				if( new_tuple_offset <= (uintptr_t) ( ((void*)(tuple_offsets + pg_cntxt->tuples_stored)) - get_page(pg_cntxt) ) )
					break;

				memcpy(get_page(pg_cntxt) + new_tuple_offset, tuples_to_insert, pg_cntxt->tuple_definition->size);

				tuple_offsets[pg_cntxt->tuples_stored] = new_tuple_offset;
				(*((uint16_t*)get_page_content(pg_cntxt)))++;
				pg_cntxt->tuples_stored++;
				num_tuples_to_insert--;
				tuples_to_insert += pg_cntxt->tuple_definition->size;
			}
			return num_tuples_to_insert;
		}
		default :
		{
			return 0;
		}
	}
}