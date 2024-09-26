#include<data_type_info.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(2));
	initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 4096, 2);
	tuple_type_info->containees[0].type_info = UINT_3_NON_NULLABLE;
	tuple_type_info->containees[1].type_info = BIT_FIELD_3_NON_NULLABLE;
	finalize_type_info(tuple_type_info);
	print_type_info(tuple_type_info);printf("\n");

	char tuple[4096];
	initialize_minimal_data_for_type_info(tuple_type_info, tuple);

	print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

	return 0;
}