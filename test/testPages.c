#include<stdio.h>

#include<alloca.h>

#include<tuple.h>
#include<page_layout.h>

#define PAGE_SIZE    1024
char page[PAGE_SIZE] = {};
//#define PAGE_LAYOUT  TUPLE_ARRAY /*SLOTTED_PAGE*/

char tuple_cache[PAGE_SIZE];

// output print string
char print_buffer[PAGE_SIZE];

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

int main()
{
	// allocate size of tuple definition
	tuple_def* def = alloca(sizeof(tuple_def) + (sizeof(element_def) * 24));

	// initialize tuple definition and insert element definitions
	init_tuple_def(def);

	insert_element_def(def,   UINT, 8);
	insert_element_def(def,    INT, 1);
	insert_element_def(def,   UINT, 1);
	insert_element_def(def, STRING, VARIABLE_SIZED);
	insert_element_def(def, STRING, 6);
	insert_element_def(def, STRING, 10);
	insert_element_def(def,  FLOAT, 8);

	finalize_tuple_def(def);

	if(is_empty_tuple_def(def))
	{
		printf("ERROR BUILDING TUPLE DEFINITION\n");
		return -1;
	}

	print_tuple_def(def);
	printf("\n\n");

	// ---------------

	void* tuple_0 = tuple_cache;

	row row_0 = {3003, -123, 21, "rohan is a good boy", "roopa", "DVIVEDI", 99.99};

	copy_element_to_tuple(def, 0, tuple_0, &(row_0.c0));
	copy_element_to_tuple(def, 1, tuple_0, &(row_0.c1));
	copy_element_to_tuple(def, 2, tuple_0, &(row_0.c2));
	copy_element_to_tuple(def, 3, tuple_0, (row_0.c3));
	copy_element_to_tuple(def, 4, tuple_0, (row_0.c4));
	copy_element_to_tuple(def, 5, tuple_0, (row_0.c5));
	copy_element_to_tuple(def, 6, tuple_0, &(row_0.c6));

	sprint_tuple(print_buffer, tuple_0, def);
	printf("tuple 0 : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple_0), print_buffer);

	insert_tuple(page, PAGE_SIZE, def, tuple_0);

	// ---------------

	void* tuple_1 = tuple_cache;

	row row_1 = {3003, -12, 16, "rohan bad boy", "rupa", "joshi", 512};

	copy_element_to_tuple(def, 0, tuple_1, &(row_1.c0));
	copy_element_to_tuple(def, 1, tuple_1, &(row_1.c1));
	copy_element_to_tuple(def, 2, tuple_1, &(row_1.c2));
	copy_element_to_tuple(def, 3, tuple_1, (row_1.c3));
	copy_element_to_tuple(def, 4, tuple_1, (row_1.c4));
	copy_element_to_tuple(def, 5, tuple_1, (row_1.c5));
	copy_element_to_tuple(def, 6, tuple_1, &(row_1.c6));

	sprint_tuple(print_buffer, tuple_1, def);
	printf("tuple 1 : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple_1), print_buffer);

	insert_tuple(page, PAGE_SIZE, def, tuple_1);

	// ---------------

	void* tuple_2 = tuple_cache;

	row row_2 = {3007, -12, 18, "rohan is awesome", "Rohi", "MOM+DAD", 2021};

	copy_element_to_tuple(def, 0, tuple_2, &(row_2.c0));
	copy_element_to_tuple(def, 1, tuple_2, &(row_2.c1));
	copy_element_to_tuple(def, 2, tuple_2, &(row_2.c2));
	copy_element_to_tuple(def, 3, tuple_2, (row_2.c3));
	copy_element_to_tuple(def, 4, tuple_2, (row_2.c4));
	copy_element_to_tuple(def, 5, tuple_2, (row_2.c5));
	copy_element_to_tuple(def, 6, tuple_2, &(row_2.c6));

	sprint_tuple(print_buffer, tuple_2, def);
	printf("tuple 2 : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple_2), print_buffer);

	insert_tuple(page, PAGE_SIZE, def, tuple_2);

	// ---------------

	void* tuple_3 = tuple_cache;

	row row_3 = {3, -53, 3, "ro", "RO", "rO", 20.21};

	copy_element_to_tuple(def, 0, tuple_3, &(row_3.c0));
	copy_element_to_tuple(def, 1, tuple_3, &(row_3.c1));
	copy_element_to_tuple(def, 2, tuple_3, &(row_3.c2));
	copy_element_to_tuple(def, 3, tuple_3, (row_3.c3));
	copy_element_to_tuple(def, 4, tuple_3, (row_3.c4));
	copy_element_to_tuple(def, 5, tuple_3, (row_3.c5));
	copy_element_to_tuple(def, 6, tuple_3, &(row_3.c6));

	sprint_tuple(print_buffer, tuple_3, def);
	printf("tuple 3 : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple_3), print_buffer);

	insert_tuple(page, PAGE_SIZE, def, tuple_3);

	// ---------------

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

	// ---------------
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	// ---------------

	delete_tuple(page, PAGE_SIZE, def, 1);

	// ---------------
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	// ---------------

	void* tuple_1u = tuple_cache;

	row row_1u = {5004, -123, 60, 
		"Project built by Rohan Dvivedi. Only coder on this project.", 
		"Roopa", "Dvivedi", 65536};

	copy_element_to_tuple(def, 0, tuple_1u, &(row_1u.c0));
	copy_element_to_tuple(def, 1, tuple_1u, &(row_1u.c1));
	copy_element_to_tuple(def, 2, tuple_1u, &(row_1u.c2));
	copy_element_to_tuple(def, 3, tuple_1u, (row_1u.c3));
	copy_element_to_tuple(def, 4, tuple_1u, (row_1u.c4));
	copy_element_to_tuple(def, 5, tuple_1u, (row_1u.c5));
	copy_element_to_tuple(def, 6, tuple_1u, &(row_1u.c6));

	sprint_tuple(print_buffer, tuple_1u, def);
	printf("tuple 1 : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple_1u), print_buffer);

	update_tuple(page, PAGE_SIZE, def, 1, tuple_1u);

	// ---------------
	
	print_all_tuples(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------
	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");

	return 0;
}