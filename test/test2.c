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

#define string_user_value(s) ((user_value){.data = s, .data_size = strlen(s)})

int main()
{
	// create tuple_def for the test cases
	tuple_def* def = get_tuple_definition();

	// init tuple
	init_tuple(def, my_tuple);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// set them all to non nulls
	printf("\nsetting all attributes to non NULLs\n");
	set_element_in_tuple(def, 0, my_tuple, &((user_value){.bit_field_value = 29}));
	set_element_in_tuple(def, 1, my_tuple, &((user_value){.large_uint_value = get_large_uint(29)}));
	set_element_in_tuple(def, 2, my_tuple, &string_user_value("Rohan"));
	set_element_in_tuple(def, 3, my_tuple, &((user_value){.int_value = -29}));
	set_element_in_tuple(def, 4, my_tuple, &((user_value){.bit_field_value = 500}));
	set_element_in_tuple(def, 5, my_tuple, &string_user_value("Dvivedi"));
	set_element_in_tuple(def, 6, my_tuple, &((user_value){.uint_value = 29}));

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// set them all to non nulls
	printf("\nsetting all attributes to different non NULLs\n");
	set_element_in_tuple(def, 0, my_tuple, &((user_value){.bit_field_value = 30}));
	set_element_in_tuple(def, 1, my_tuple, &((user_value){.large_uint_value = get_large_uint(10)}));
	set_element_in_tuple(def, 2, my_tuple, &string_user_value("Rohan V"));
	set_element_in_tuple(def, 3, my_tuple, &((user_value){.int_value = -50}));
	set_element_in_tuple(def, 4, my_tuple, &((user_value){.bit_field_value = 250}));
	set_element_in_tuple(def, 5, my_tuple, &string_user_value("V Dvivedi"));
	set_element_in_tuple(def, 6, my_tuple, &((user_value){.uint_value = 3000}));

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);


	int cmp = compare_tuples(
							my_tuple, def, ((uint32_t[]){0}),
							my_tuple, def, ((uint32_t[]){1}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	// set them all to non nulls
	printf("\nsetting all attributes to NULLs\n");
	set_element_in_tuple(def, 0, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 1, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 2, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 3, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 4, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 5, my_tuple, NULL_USER_VALUE);
	set_element_in_tuple(def, 6, my_tuple, NULL_USER_VALUE);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// delete tuple_def
	delete_tuple_def(def);

	return 0;
}