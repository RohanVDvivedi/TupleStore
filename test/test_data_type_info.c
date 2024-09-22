#include<data_type_info.h>

#include<stdio.h>

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
	return 0;
}