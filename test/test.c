#include<stdio.h>
#include<stdlib.h>

#include<alloca.h>

#include<tuple.h>
#include<page_layout.h>

// comment the below macro to test the SLOTTED_PAGE_LAYOUT
#define TEST_FIXED_ARRAY_PAGE_LAYOUT

#define PAGE_SIZE    1024
char page[PAGE_SIZE] = {};
//#define PAGE_LAYOUT  TUPLE_ARRAY /*SLOTTED_PAGE*/

void init_tuple_definition(tuple_def* def)
{
	// initialize tuple definition and insert element definitions
	init_tuple_def(def);

	insert_element_def(def,   UINT, 8);
	insert_element_def(def,    INT, 1);
	insert_element_def(def,   UINT, 1);

	#ifndef TEST_FIXED_ARRAY_PAGE_LAYOUT
		insert_element_def(def, STRING, VARIABLE_SIZED);
	#endif
	
	insert_element_def(def, STRING, 6);
	insert_element_def(def, STRING, 10);
	insert_element_def(def,  FLOAT, 8);

	finalize_tuple_def(def);

	if(is_empty_tuple_def(def))
	{
		printf("ERROR BUILDING TUPLE DEFINITION\n");
		exit(-1);
	}

	print_tuple_def(def);
	printf("\n\n");
}

// a row like struct for ease in building test tuples
typedef struct row row;
struct row
{
	u8 c0;
	i1 c1;
	u1 c2;
	char* c3;
	char* c4;
	char* c5;
	f8 c6;
};

void build_tuple_from_row_struct(const tuple_def* def, void* tuple, const row* r)
{
	int column_no = 0;

	copy_element_to_tuple(def, column_no++, tuple, &(r->c0));
	copy_element_to_tuple(def, column_no++, tuple, &(r->c1));
	copy_element_to_tuple(def, column_no++, tuple, &(r->c2));

	#ifndef TEST_FIXED_ARRAY_PAGE_LAYOUT
		copy_element_to_tuple(def, column_no++, tuple, (r->c3));
	#endif
	
	copy_element_to_tuple(def, column_no++, tuple, (r->c4));
	copy_element_to_tuple(def, column_no++, tuple, (r->c5));
	copy_element_to_tuple(def, column_no++, tuple, &(r->c6));

	// output print string
	char print_buffer[PAGE_SIZE];

	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);
}

int main()
{
	// allocate size of tuple definition
	tuple_def* def = alloca(sizeof(tuple_def) + (sizeof(element_def) * 24));

	// initialize tuple definition and insert element definitions
	init_tuple_definition(def);

	// ---------------	DECLARE TEMP variables

	// to build intermediate tuples (only 1 at a time)
	char tuple_cache[PAGE_SIZE];
	// and
	row* r = NULL;

	// ---------------	INSERT

	r = &(row){3003, -123, 21, "rohan is a good boy", "roopa", "DVIVEDI", 99.99};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	// ---------------	INSERT

	r = &(row){3003, -12, 16, "rohan bad boy", "rupa", "joshi", 512};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	// ---------------`INSERT

	r = &(row){3007, -12, 18, "rohan is awesome", "Rohi", "MOM+DAD", 2021};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	// ---------------	INSERT

	r = &(row){3, -53, 3, "ro", "RO", "rO", 20.21};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	// ---------------	COMPARE 2 TUPLES

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(	seek_to_nth_tuple(page, PAGE_SIZE, def, 0), 
								seek_to_nth_tuple(page, PAGE_SIZE, def, 1), 
								def)
			);

	printf("compare(tuple_2 , tuple_1) = %d\n\n", 
				compare_tuples(	seek_to_nth_tuple(page, PAGE_SIZE, def, 2), 
								seek_to_nth_tuple(page, PAGE_SIZE, def, 1), 
								def)
			);

	// ---------------	PRINT TUPLES
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	// --------------- DELETE

	delete_tuple(page, PAGE_SIZE, def, 1);

	// ---------------	PRINT TUPLES
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){5004, -123, 65, 
		"Project built by Rohan Dvivedi. The only coder on this project.", 
		"Roopa", "Dvivedi", 65536};

	build_tuple_from_row_struct(def, tuple_cache, r);

	update_tuple(page, PAGE_SIZE, def, 1, tuple_cache);

	// ---------------	PRINT TUPLES
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	return 0;
}