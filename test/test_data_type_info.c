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
	return 0;
}