#include<data_type_info.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	{
		data_type_info s1 = get_fixed_length_string_type(10, 0);
		data_type_info s2 = get_fixed_length_string_type(12, 1);
		data_type_info s3 = get_variable_length_string_type(100);
		data_type_info s4 = get_variable_length_string_type(300);

		data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(8));
		initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 4096, 8);
		tuple_type_info->containees[0].type_info = UINT_3_NON_NULLABLE;
		tuple_type_info->containees[1].type_info = BIT_FIELD_5_NON_NULLABLE;
		tuple_type_info->containees[2].type_info = &s3;
		tuple_type_info->containees[3].type_info = &s1;
		tuple_type_info->containees[4].type_info = &s2;
		tuple_type_info->containees[5].type_info = &s4;
		tuple_type_info->containees[6].type_info = FLOAT_double_NULLABLE;
		tuple_type_info->containees[7].type_info = BIT_FIELD_5_NULLABLE;
		finalize_type_info(tuple_type_info);
		print_type_info(tuple_type_info);printf("\n");

		char tuple[4096];
		initialize_minimal_data_for_type_info(tuple_type_info, tuple);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (user_value){.uint_value = 5});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (user_value){.bit_field_value = 0x6});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 100, (user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (user_value){.string_value = "ABC", .string_size = strlen("ABC")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, (user_value){.string_value = "DEF", .string_size = strlen("DEF")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 300, (user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, (user_value){.double_value = -5});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, (user_value){.bit_field_value = 0x36});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (user_value){.uint_value = 99});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (user_value){.bit_field_value = 0x25});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 100, (user_value){.string_value = "Manan Joshi", .string_size = strlen("Manan Joshi")});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (user_value){.string_value = "GHIJK", .string_size = strlen("GHIJK")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, (user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 300, (user_value){.string_value = "Vipulkumar Dvivedi", .string_size = strlen("Vipulkumar Dvivedi")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, (user_value){.double_value = 55});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, (user_value){.bit_field_value = 0x25});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, (*NULL_USER_VALUE));

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, BIT_FIELD_4_NULLABLE);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (user_value){.bit_field_value = 0x1});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (user_value){.bit_field_value = 0x2});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (user_value){.bit_field_value = 0x3});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (user_value){.bit_field_value = 0x4});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (user_value){.bit_field_value = 0x5});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (user_value){.bit_field_value = 0x6});


		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (*NULL_USER_VALUE));

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, LARGE_UINT_4_NULLABLE);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (user_value){.large_uint_value = get_uint256(12)});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (user_value){.large_uint_value = get_uint256(13)});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (user_value){.large_uint_value = get_uint256(14)});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (user_value){.large_uint_value = get_uint256(15)});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (user_value){.large_uint_value = get_uint256(16)});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (user_value){.large_uint_value = get_uint256(17)});


		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (*NULL_USER_VALUE));

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type(300);
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 500, 1, &s4);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, (user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, (user_value){.string_value = "Rupa Joshi", .string_size = strlen("Rupa Joshi")});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, (user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, (user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, (user_value){.string_value = "Rohan Vipulkumar Dvivedi", .string_size = strlen("Rohan Vipulkumar Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, (user_value){.string_value = "Devashree Joshi", .string_size = strlen("Devashree Joshi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, (*NULL_USER_VALUE));
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, (*NULL_USER_VALUE));

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, UINT_5_NULLABLE);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, (user_value){.uint_value = 12});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, (user_value){.uint_value = 13});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, (user_value){.uint_value = 14});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, (user_value){.uint_value = 15});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	return 0;
}