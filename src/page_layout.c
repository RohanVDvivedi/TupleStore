#include<page_layout.h>

#include<tuple.h>

// -------------------------------------------
// UTILITY FUNCTIONS         SLOTTED PAGE TYPE
// -------------------------------------------

// -------------------------------------------
// -------------------------------------------

// -------------------------------------------
// UTILITY FUNCTIONS     FIXED_ARRAY PAGE TYPE
// -------------------------------------------

static uint16_t get_tuple_capacity_FIXED_ARRAY(uint32_t page_size, uint32_t tuple_size)
{
	// unsigned int divisions give floor results by defaults
	return ((page_size - sizeof(uint16_t)) * 8) / (8 * tuple_size + 1); 
}

// -------------------------------------------
// -------------------------------------------

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index, const void* external_tuple)
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

int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
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

int is_deleted_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
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

uint16_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return *((const uint16_t*)page);
}

void* seek_to_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
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

int can_accomodate_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
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

uint32_t get_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
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

int compact_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
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

void print_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
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