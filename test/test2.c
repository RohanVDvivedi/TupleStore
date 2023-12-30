#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<tuple.h>
#include<page_layout.h>

#define MAX_TUPLE_SIZE 4096
char my_tuple[MAX_TUPLE_SIZE];

tuple_def* get_tuple_definition()
{
	// initialize tuple definition and insert element definitions
	tuple_def* def = get_new_tuple_def("my_table", 16, MAX_TUPLE_SIZE);

	int res = insert_element_def(def, "col_0", BIT_FIELD, 5, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 0\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_1", LARGE_UINT, 5, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 1\n");
		exit(-1);
	}

	res = insert_element_def(def, "var_col_2", VAR_STRING, 2, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 2\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_3", INT, 3, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 3\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_4", BIT_FIELD, 9, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 4\n");
		exit(-1);
	}

	res = insert_element_def(def, "var_col_5", VAR_STRING, 2, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 5\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_6", UINT, 5, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 6\n");
		exit(-1);
	}

	res = finalize_tuple_def(def);
	if(res == 0)
	{
		printf("failed finalizing tuple definition\n");
		exit(-1);
	}

	if(is_empty_tuple_def(def))
	{
		printf("ERROR BUILDING TUPLE DEFINITION\n");
		exit(-1);
	}

	print_tuple_def(def);
	printf("\n\n");

	return def;
}

int main()
{
	// create tuple_def for the test cases
	tuple_def* def = get_tuple_definition();

	// init tuple
	init_tuple(def, my_tuple);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// delete tuple_def
	delete_tuple_def(def);

	return 0;
}