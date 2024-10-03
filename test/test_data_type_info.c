#include<data_type_info.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	{
		data_type_info s1 = get_fixed_length_string_type("STRING", 10, 0);
		data_type_info s2 = get_fixed_length_string_type("STRING", 12, 1);
		data_type_info s3 = get_variable_length_string_type("STRING", 100);
		data_type_info s4 = get_variable_length_string_type("STRING", 300);

		data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(8));
		initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 1024, 8);
		tuple_type_info->containees[0].type_info = UINT_NON_NULLABLE[3];
		tuple_type_info->containees[1].type_info = BIT_FIELD_NON_NULLABLE[5];
		tuple_type_info->containees[2].type_info = &s3;
		tuple_type_info->containees[3].type_info = &s1;
		tuple_type_info->containees[4].type_info = &s2;
		tuple_type_info->containees[5].type_info = &s4;
		tuple_type_info->containees[6].type_info = FLOAT_double_NULLABLE;
		tuple_type_info->containees[7].type_info = BIT_FIELD_NULLABLE[5];
		finalize_type_info(tuple_type_info);
		print_type_info(tuple_type_info);printf("\n");

		char tuple[256];
		initialize_minimal_data_for_type_info(tuple_type_info, tuple);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, &(user_value){.uint_value = 5});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, &(user_value){.bit_field_value = 0x6});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 100, &(user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, &(user_value){.string_value = "ABC", .string_size = strlen("ABC")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, &(user_value){.string_value = "DEF", .string_size = strlen("DEF")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 300, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, &(user_value){.double_value = -5});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, &(user_value){.bit_field_value = 0x36});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		const void* e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3);
		const void* e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, &(user_value){.uint_value = 99});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, &(user_value){.bit_field_value = 0x25});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 100, &(user_value){.string_value = "Manan Joshi", .string_size = strlen("Manan Joshi")});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, &(user_value){.string_value = "GHIJK", .string_size = strlen("GHIJK")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 300, &(user_value){.string_value = "Vipulkumar DvivediXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX44444444444444444444444444444444444111111111111111111111111111111111111111111111111111111110", .string_size = strlen("Vipulkumar DvivediXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX44444444444444444444444444444444444111111111111111111111111111111111111111111111111111111110")});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, &(user_value){.double_value = 55});
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, &(user_value){.bit_field_value = 0x25});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, EMPTY_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3);
		e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3);
		e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container(tuple_type_info, tuple, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 2, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 3, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 4, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 5, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 6, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(tuple_type_info, tuple, 7, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, BIT_FIELD_NULLABLE[4]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, &(user_value){.bit_field_value = 0x1});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, &(user_value){.bit_field_value = 0x2});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, &(user_value){.bit_field_value = 0x3});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, &(user_value){.bit_field_value = 0x4});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, &(user_value){.bit_field_value = 0x5});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, &(user_value){.bit_field_value = 0x6});


		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, LARGE_UINT_NULLABLE[4]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, &(user_value){.large_uint_value = get_uint256(12)});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, &(user_value){.large_uint_value = get_uint256(13)});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, &(user_value){.large_uint_value = get_uint256(14)});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, &(user_value){.large_uint_value = get_uint256(15)});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, &(user_value){.large_uint_value = get_uint256(16)});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, &(user_value){.large_uint_value = get_uint256(17)});


		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("STRING", 300);
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 500, 1, &s4);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, &(user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, &(user_value){.string_value = "Rupa Joshi", .string_size = strlen("Rupa Joshi")});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.string_value = "Rohan Vipulkumar Dvivedi", .string_size = strlen("Rohan Vipulkumar Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, &(user_value){.string_value = "Devashree Joshi", .string_size = strlen("Devashree Joshi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, BIT_FIELD_NULLABLE[5]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, &(user_value){.bit_field_value = 0x12});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, &(user_value){.bit_field_value = 0x16});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.bit_field_value = 0x04});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.bit_field_value = 0x15});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.bit_field_value = 0x15});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.bit_field_value = 0x08});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, UINT_NULLABLE[5]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, &(user_value){.uint_value = 12});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, &(user_value){.uint_value = 13});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.uint_value = 14});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.uint_value = 15});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.uint_value = 16});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.uint_value = 17});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("STRING", 300);
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, &s4);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 0, 50, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 1, 50, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});
		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.string_value = "Devashree Joshi", .string_size = strlen("Devashree Joshi")});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.string_value = "Vipulkumar Dvivedi", .string_size = strlen("Vipulkumar Dvivedi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container(array_type_info, array, 2, 50, &(user_value){.string_value = "Manan Joshi", .string_size = strlen("Manan Joshi")});
		set_user_value_to_containee_in_container(array_type_info, array, 4, 50, &(user_value){.string_value = "Avyaan Joshi", .string_size = strlen("Avyaan Joshi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("STRING", 300);
		finalize_type_info(&s4);
		print_type_info(&s4);printf("\n");

		char s[4096];
		set_user_value_for_type_info(&s4, s, 0, 300, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});

		print_data_for_data_type_info(&s4, s);printf("\n");

		expand_container(&s4, s, 6, 11, 200);

		print_data_for_data_type_info(&s4, s);printf("\n");

		char* to_add = "Vipulkumar ";
		for(char* c = to_add; (*c) != 0; c++)
			set_user_value_to_containee_in_container(&s4, s, 6 + (c - to_add), 300, &(user_value){.uint_value = (*c)});

		print_data_for_data_type_info(&s4, s);printf("\n");

		discard_from_container(&s4, s, 11, 5);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));

		set_user_value_for_type_info(&s4, s, 0, 300, EMPTY_USER_VALUE);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));

		int setting_to_null = set_user_value_for_type_info(&s4, s, 0, 300, NULL_USER_VALUE);

		printf("tried setting to null, result = %d\n", setting_to_null);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));
	}

	return 0;
}