#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<tuple.h>
#include<page_layout.h>

#include<test_serde_for_type_info.h>

#define MAX_TUPLE_SIZE 4096
char my_tuple[MAX_TUPLE_SIZE];

tuple_def tuple_definition;
char tuple_type_info_memory[sizeof_tuple_data_type_info(9)];
data_type_info* tuple_type_info = (data_type_info*)tuple_type_info_memory;
data_type_info c2_type_info;
data_type_info c5_type_info;

tuple_def* get_tuple_definition()
{
	// initialize tuple definition and insert element definitions
	initialize_tuple_data_type_info(tuple_type_info, "my_table", 1, MAX_TUPLE_SIZE, 9);

	strcpy(tuple_type_info->containees[0].field_name, "col_0");
	tuple_type_info->containees[0].al.type_info = BIT_FIELD_NULLABLE[5];

	strcpy(tuple_type_info->containees[1].field_name, "col_1");
	tuple_type_info->containees[1].al.type_info = LARGE_UINT_NULLABLE[5];

	c2_type_info = get_variable_length_string_type("", 1 << (2 * 8));
	strcpy(tuple_type_info->containees[2].field_name, "var_col_2");
	tuple_type_info->containees[2].al.type_info = &c2_type_info;

	strcpy(tuple_type_info->containees[3].field_name, "col_3");
	tuple_type_info->containees[3].al.type_info = INT_NULLABLE[3];

	strcpy(tuple_type_info->containees[4].field_name, "col_4");
	tuple_type_info->containees[4].al.type_info = BIT_FIELD_NULLABLE[9];

	c5_type_info = get_variable_length_string_type("", 1 << (2 * 8));
	strcpy(tuple_type_info->containees[5].field_name, "var_col_5");
	tuple_type_info->containees[5].al.type_info = &c5_type_info;

	strcpy(tuple_type_info->containees[6].field_name, "col_6");
	tuple_type_info->containees[6].al.type_info = UINT_NULLABLE[5];

	strcpy(tuple_type_info->containees[7].field_name, "col_7");
	tuple_type_info->containees[7].al.type_info = FLOAT_long_double_NULLABLE;

	strcpy(tuple_type_info->containees[8].field_name, "col_8");
	tuple_type_info->containees[8].al.type_info = LARGE_UINT_NULLABLE[4];

	if(!initialize_tuple_def(&tuple_definition, tuple_type_info))
	{
		printf("failed finalizing tuple definition\n");
		exit(-1);
	}

	print_tuple_def(&tuple_definition);
	printf("\n\n");

	test_serde_for_type_info(tuple_type_info);

	return &tuple_definition;
}

#define string_user_value(s) ((user_value){.string_value = s, .string_size = strlen(s)})

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
	set_element_in_tuple(def, STATIC_POSITION(0), my_tuple, &((user_value){.bit_field_value = 29}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(1), my_tuple, &((user_value){.large_uint_value = get_uint256(0x29)}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(2), my_tuple, &string_user_value("Rohan"), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(3), my_tuple, &((user_value){.int_value = -29}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(4), my_tuple, &((user_value){.bit_field_value = 500}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(5), my_tuple, &string_user_value("Dvivedi"), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(6), my_tuple, &((user_value){.uint_value = 29}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(7), my_tuple, &((user_value){.long_double_value = 2900}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(8), my_tuple, &((user_value){.large_uint_value = get_uint256(0x45)}), UINT32_MAX);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// set them all to non nulls
	printf("\nsetting all attributes to different non NULLs\n");
	set_element_in_tuple(def, STATIC_POSITION(0), my_tuple, &((user_value){.bit_field_value = 30}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(1), my_tuple, &((user_value){.large_uint_value = get_uint256(0x10)}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(2), my_tuple, &string_user_value("Rohan V"), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(3), my_tuple, &((user_value){.int_value = 30}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(4), my_tuple, &((user_value){.bit_field_value = 250}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(5), my_tuple, &string_user_value("V Dvivedi"), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(6), my_tuple, &((user_value){.uint_value = 3000}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(7), my_tuple, &((user_value){.long_double_value = 295.2966}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(8), my_tuple, &((user_value){.large_uint_value = get_uint256(0x99)}), UINT32_MAX);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	set_element_in_tuple_from_tuple(def, STATIC_POSITION(8), my_tuple, def, STATIC_POSITION(1), my_tuple, UINT32_MAX);

	// print the tuple
	printf("\nBuilt tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	int cmp = compare_tuples(
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(0)}),
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(1)}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	cmp = compare_tuples(
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(0)}),
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(3)}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	cmp = compare_tuples(
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(0)}),
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(7)}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	cmp = compare_tuples(
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(1)}),
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(3)}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	cmp = compare_tuples(
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(3)}),
							my_tuple, def, ((positional_accessor[]){STATIC_POSITION(1)}),
							((compare_direction[]){ASC}), 1);
	printf("cmp = %d\n", cmp);

	// set them all to non nulls
	printf("\nsetting all attributes to NULLs\n");
	set_element_in_tuple(def, STATIC_POSITION(0), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(1), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(2), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(3), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(4), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(5), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(6), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(7), my_tuple, NULL_USER_VALUE, 0);
	set_element_in_tuple(def, STATIC_POSITION(8), my_tuple, NULL_USER_VALUE, 0);

	// print the tuple
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, my_tuple));
	print_tuple(my_tuple, def);

	// delete tuple_def

	return 0;
}