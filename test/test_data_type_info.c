#include<data_type_info.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(4));
	initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 4096, 4);
	tuple_type_info->containees[0].type_info = UINT_3_NON_NULLABLE;
	tuple_type_info->containees[1].type_info = BIT_FIELD_5_NON_NULLABLE;
	tuple_type_info->containees[2].type_info = INT_3_NULLABLE;
	tuple_type_info->containees[3].type_info = BIT_FIELD_5_NULLABLE;
	finalize_type_info(tuple_type_info);
	print_type_info(tuple_type_info);printf("\n");

	char tuple[4096];
	initialize_minimal_data_for_type_info(tuple_type_info, tuple);

	print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

	set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (user_value){.uint_value = 5});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (user_value){.bit_field_value = 0x6});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, (user_value){.int_value = -5});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (user_value){.bit_field_value = 0x36});

	print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

	set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (user_value){.uint_value = 99});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (user_value){.bit_field_value = 0x25});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, (user_value){.int_value = 55});
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (user_value){.bit_field_value = 0x25});

	print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

	set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (*NULL_USER_VALUE));
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (*NULL_USER_VALUE));
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, (*NULL_USER_VALUE));
	set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (*NULL_USER_VALUE));

	print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

	return 0;
}