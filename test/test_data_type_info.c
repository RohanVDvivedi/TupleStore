#include<data_type_info.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	finalize_type_info(BIT_FIELD_33_NULLABLE);
	print_type_info(BIT_FIELD_33_NULLABLE);
	printf("\n");

	finalize_type_info(BIT_FIELD_33_NON_NULLABLE);
	print_type_info(BIT_FIELD_33_NON_NULLABLE);
	printf("\n");

	finalize_type_info(UINT_5_NULLABLE);
	print_type_info(UINT_5_NULLABLE);
	printf("\n");

	finalize_type_info(UINT_5_NON_NULLABLE);
	print_type_info(UINT_5_NON_NULLABLE);
	printf("\n");

	finalize_type_info(INT_5_NULLABLE);
	print_type_info(INT_5_NULLABLE);
	printf("\n");

	finalize_type_info(INT_5_NON_NULLABLE);
	print_type_info(INT_5_NON_NULLABLE);
	printf("\n");

	finalize_type_info(FLOAT_double_NULLABLE);
	print_type_info(FLOAT_double_NULLABLE);
	printf("\n");

	finalize_type_info(FLOAT_long_double_NON_NULLABLE);
	print_type_info(FLOAT_long_double_NON_NULLABLE);
	printf("\n");

	finalize_type_info(LARGE_UINT_21_NULLABLE);
	print_type_info(LARGE_UINT_21_NULLABLE);
	printf("\n");

	finalize_type_info(LARGE_UINT_21_NON_NULLABLE);
	print_type_info(LARGE_UINT_21_NON_NULLABLE);
	printf("\n");

	data_type_info fixed_length_string = get_fixed_length_string_type(12, 1);
	finalize_type_info(&fixed_length_string);
	print_type_info(&fixed_length_string);
	printf("\n");

	data_type_info fixed_length_blob = get_fixed_length_blob_type(15, 1);
	finalize_type_info(&fixed_length_blob);
	print_type_info(&fixed_length_blob);
	printf("\n");

	data_type_info fixed_length_string_nn = get_fixed_length_string_type(13, 0);
	finalize_type_info(&fixed_length_string_nn);
	print_type_info(&fixed_length_string_nn);
	printf("\n");

	data_type_info fixed_length_blob_nn = get_fixed_length_blob_type(10, 0);
	finalize_type_info(&fixed_length_blob_nn);
	print_type_info(&fixed_length_blob_nn);
	printf("\n");

	data_type_info var_length_string = get_variable_length_string_type(1000000);
	finalize_type_info(&var_length_string);
	print_type_info(&var_length_string);
	printf("\n");

	data_type_info var_length_blob = get_variable_length_blob_type(1000);
	finalize_type_info(&var_length_blob);
	print_type_info(&var_length_blob);
	printf("\n");

	data_type_info* fixed_sized_tuple = alloca(sizeof_tuple_data_type_info(4));
	initialize_tuple_data_type_info(fixed_sized_tuple, "fixed_sized_tuple", 1, 0, 4);
	strcpy(fixed_sized_tuple->containees[0].field_name, "a");
	fixed_sized_tuple->containees[0].type_info = INT_7_NULLABLE;
	strcpy(fixed_sized_tuple->containees[1].field_name, "b");
	fixed_sized_tuple->containees[1].type_info = UINT_6_NON_NULLABLE;
	strcpy(fixed_sized_tuple->containees[2].field_name, "c");
	fixed_sized_tuple->containees[2].type_info = LARGE_UINT_15_NULLABLE;
	strcpy(fixed_sized_tuple->containees[3].field_name, "d");
	fixed_sized_tuple->containees[3].type_info = BIT_FIELD_15_NULLABLE;
	finalize_type_info(fixed_sized_tuple);
	print_type_info(fixed_sized_tuple);
	printf("\n");

	data_type_info* var_sized_tuple = alloca(sizeof_tuple_data_type_info(6));
	initialize_tuple_data_type_info(var_sized_tuple, "var_sized_tuple", 0, 1000000, 6);
	strcpy(var_sized_tuple->containees[0].field_name, "a");
	var_sized_tuple->containees[0].type_info = INT_7_NULLABLE;
	strcpy(var_sized_tuple->containees[1].field_name, "b");
	var_sized_tuple->containees[1].type_info = UINT_6_NON_NULLABLE;
	strcpy(var_sized_tuple->containees[2].field_name, "c");
	var_sized_tuple->containees[2].type_info = &var_length_string;
	strcpy(var_sized_tuple->containees[3].field_name, "d");
	var_sized_tuple->containees[3].type_info = BIT_FIELD_7_NULLABLE;
	strcpy(var_sized_tuple->containees[4].field_name, "e");
	var_sized_tuple->containees[4].type_info = LARGE_UINT_15_NULLABLE;
	strcpy(var_sized_tuple->containees[5].field_name, "f");
	var_sized_tuple->containees[5].type_info = &var_length_blob;
	finalize_type_info(var_sized_tuple);
	print_type_info(var_sized_tuple);
	printf("\n");

	data_type_info a1 = get_fixed_element_count_array_type("a1", 3, 0, 1, UINT_4_NULLABLE);
	finalize_type_info(&a1);
	print_type_info(&a1);
	printf("\n");

	a1 = get_fixed_element_count_array_type("a1", 3, 0, 1, BIT_FIELD_4_NULLABLE);
	finalize_type_info(&a1);
	print_type_info(&a1);
	printf("\n");

	a1 = get_fixed_element_count_array_type("a1", 4, 1000000, 0, &var_length_blob);
	finalize_type_info(&a1);
	print_type_info(&a1);
	printf("\n");

	return 0;
}