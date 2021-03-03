#include<stdio.h>

#include<alloca.h>

#include<tuple.h>

#define PAGE_SIZE    1024
//#define PAGE_LAYOUT  TUPLE_ARRAY /*SLOTTED_PAGE*/

// output print string
char print_buffer[1024];

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
	insert_element_def(def, STRING, VARIABLE_SIZED /*24*/);
	insert_element_def(def, STRING, 6);
	insert_element_def(def, STRING, 10);
	insert_element_def(def,  FLOAT, 8);
	
	finalize_tuple_def(def);

	print_tuple_def(def);
	printf("\n\n");

	// this is your single lonely datapage
	void* page = alloca(PAGE_SIZE);

	// ---------------

	void* tuple_0 = page + 0;

	row row_0 = {3003, -123, 21, "rohan is a good boy", "roopa", "DVIVEDI", 99.99};

	copy_element_to_tuple(def, 0, tuple_0, &(row_0.c0));
	copy_element_to_tuple(def, 1, tuple_0, &(row_0.c1));
	copy_element_to_tuple(def, 2, tuple_0, &(row_0.c2));
	copy_element_to_tuple(def, 3, tuple_0, (row_0.c3));
	copy_element_to_tuple(def, 4, tuple_0, (row_0.c4));
	copy_element_to_tuple(def, 5, tuple_0, (row_0.c5));
	copy_element_to_tuple(def, 6, tuple_0, &(row_0.c6));

	sprint_tuple(print_buffer, tuple_0, def);
	printf("tuple 1 : \n\t%s\n\n", print_buffer);

	// ---------------

	void* tuple_1 = page + 512;

	row row_1 = {3003, -123, 21, "rohan is a good boy", "roopa", "DVIVEDI", 99.99};

	copy_element_to_tuple(def, 0, tuple_1, &(row_1.c0));
	copy_element_to_tuple(def, 1, tuple_1, &(row_1.c1));
	copy_element_to_tuple(def, 2, tuple_1, &(row_1.c2));
	copy_element_to_tuple(def, 3, tuple_1, (row_1.c3));
	copy_element_to_tuple(def, 4, tuple_1, (row_1.c4));
	copy_element_to_tuple(def, 5, tuple_1, (row_1.c5));
	copy_element_to_tuple(def, 6, tuple_1, &(row_1.c6));

	sprint_tuple(print_buffer, tuple_1, def);
	printf("tuple 2 : \n\t%s\n\n", print_buffer);

	// ---------------

	printf("compare(tuple_0 , tuple_1) = %d\n\n", compare_tuples(tuple_0, tuple_1, def));

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