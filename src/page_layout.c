#include<page_layout.h>

#include<tuple.h>

int insert_tuple(void* page, uint64_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

int insert_tuple_anywhere(void* page, uint64_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

int delete_tuple(void* page, uint64_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

int is_deleted_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

uint16_t get_index_for_new_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

void* seek_to_nth_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return NULL;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return NULL;
	}
}

int can_accomodate_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

uint64_t get_free_space(const void* page, uint64_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

int compact_page(const void* page, uint64_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
		return 0;
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
		return 0;
	}
}

void print_all_tuples(const void* page, uint64_t page_size, const tuple_def* tpl_d)
{
	if(tpl_d->size == VARIABLE_SIZED)
	{	
		// case : SLOTTED PAGE
		// TODO
	}
	else
	{
		// case : FIXED ARRAY PAGE
		// TODO
	}
}