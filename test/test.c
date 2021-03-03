#include<stdio.h>

#include<alloca.h>

#include<tuple.h>

#define PAGE_SIZE    4096
#define PAGE_LAYOUT  TUPLE_ARRAY /*SLOTTED_PAGE*/

// output print string
char string[1024];

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
	insert_element_def(def, STRING, 6);
	insert_element_def(def,  FLOAT, 8);
	
	finalize_tuple_def(def);

	print_tuple_def(def);

	// this is your single lonely datapage
	/*void* page = alloca(PAGE_SIZE);

	
	for(int i = 0; i < PAGE_SIZE; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n");
	*/

	return 0;
}