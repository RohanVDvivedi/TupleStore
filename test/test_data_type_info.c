#include<data_type_info.h>
#include<tuple.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<test_serde_for_type_info.h>

#include<preorder_print.h>

int set_user_value_to_containee_in_container_WRAPPER(const data_type_info* dti, void* tuple, uint32_t index, uint32_t max_size_increment_allowed, const user_value* uval)
{
	data_positional_info temp = INVALID_DATA_POSITIONAL_INFO;
	return set_user_value_to_containee_in_container(dti, tuple, index, max_size_increment_allowed, uval, &temp);
}

int main()
{
	data_positional_info temp;
	{
		data_type_info s1 = get_fixed_length_string_type("", 10, 0);
		data_type_info s2 = get_fixed_length_string_type("", 12, 1);
		data_type_info s3 = get_variable_length_string_type("", 100);
		data_type_info s4 = get_variable_length_string_type("", 300);

		data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(8));
		initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 1024, 8);
		tuple_type_info->containees[0].al.type_info = UINT_NON_NULLABLE[3];
		tuple_type_info->containees[1].al.type_info = BIT_FIELD_NON_NULLABLE[5];
		tuple_type_info->containees[2].al.type_info = &s3;
		tuple_type_info->containees[3].al.type_info = &s1;
		tuple_type_info->containees[4].al.type_info = &s2;
		tuple_type_info->containees[5].al.type_info = &s4;
		tuple_type_info->containees[6].al.type_info = FLOAT_double_NULLABLE;
		tuple_type_info->containees[7].al.type_info = BIT_FIELD_NULLABLE[5];
		for(int i = 0; i < 8; i++)
			strcpy(tuple_type_info->containees[i].field_name, "");
		finalize_type_info(tuple_type_info);
		print_type_info(tuple_type_info);printf("\n");

		test_serde_for_type_info(tuple_type_info);

		char tuple[256];
		initialize_minimal_data_for_type_info(tuple_type_info, tuple);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 0, 0, &(user_value){.uint_value = 5});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 1, 0, &(user_value){.bit_field_value = 0x6});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 2, 100, &(user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d\n", is_minimal_data_for_type_info(tuple_type_info, tuple));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 3, 0, &(user_value){.string_value = "ABC", .string_size = strlen("ABC")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 4, 0, &(user_value){.string_value = "DEF", .string_size = strlen("DEF")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 5, 300, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 6, 0, &(user_value){.double_value = -5});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 7, 0, &(user_value){.bit_field_value = 0x36});

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");

		temp = INVALID_DATA_POSITIONAL_INFO;
		const void* e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3, &temp);
		temp = INVALID_DATA_POSITIONAL_INFO;
		const void* e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4, &temp);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 0, 0, &(user_value){.uint_value = 99});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 1, 0, &(user_value){.bit_field_value = 0x25});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 2, 100, &(user_value){.string_value = "Manan Joshi", .string_size = strlen("Manan Joshi")});

		print_data_for_data_type_info(tuple_type_info, tuple);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, tuple_type_info, tuple);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, tuple_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, tuple_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(tuple, tpl_d, NULL, FNV_64_TUPLE_HASHER, tuple_type_info->element_count),
				hash_tuple(tuple, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 3, 0, &(user_value){.string_value = "GHIJK", .string_size = strlen("GHIJK")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 4, 0, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 5, 300, &(user_value){.string_value = "Vipulkumar DvivediXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX44444444444444444444444444444444444111111111111111111111111111111111111111111111111111111110", .string_size = strlen("Vipulkumar DvivediXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX44444444444444444444444444444444444111111111111111111111111111111111111111111111111111111110")});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 6, 0, &(user_value){.double_value = 55});
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 7, 0, &(user_value){.bit_field_value = 0x25});

		print_data_for_data_type_info(tuple_type_info, tuple);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, tuple_type_info, tuple);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, tuple_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, tuple_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(tuple, tpl_d, NULL, FNV_64_TUPLE_HASHER, tuple_type_info->element_count),
				hash_tuple(tuple, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 0, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 1, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 2, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 3, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 4, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 5, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 6, 0, EMPTY_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 7, 0, EMPTY_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d", is_minimal_data_for_type_info(tuple_type_info, tuple));
		{
			user_value uval;
			get_user_value_for_type_info(&uval, tuple_type_info, tuple);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, tuple_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, tuple_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(tuple, tpl_d, NULL, FNV_64_TUPLE_HASHER, tuple_type_info->element_count),
				hash_tuple(tuple, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		temp = INVALID_DATA_POSITIONAL_INFO;
		e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3, &temp);
		temp = INVALID_DATA_POSITIONAL_INFO;
		e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4, &temp);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 2, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 3, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 4, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 5, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 6, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 7, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("    is minimal = %d", is_minimal_data_for_type_info(tuple_type_info, tuple));
		{
			user_value uval;
			get_user_value_for_type_info(&uval, tuple_type_info, tuple);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, tuple_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, tuple_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(tuple, tpl_d, NULL, FNV_64_TUPLE_HASHER, tuple_type_info->element_count),
				hash_tuple(tuple, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		temp = INVALID_DATA_POSITIONAL_INFO;
		e3 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 3, &temp);
		temp = INVALID_DATA_POSITIONAL_INFO;
		e4 = get_pointer_to_containee_from_container(tuple_type_info, tuple, 4, &temp);
		printf("is 3rd element minimal = %d\n", e3 == NULL ? 0 : is_minimal_data_for_type_info(&s1, e3));
		printf("is 4th element minimal = %d\n", e4 == NULL ? 0 : is_minimal_data_for_type_info(&s2, e4));

		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 2, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 3, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 4, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 5, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 6, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(tuple_type_info, tuple, 7, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(tuple_type_info, tuple);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, BIT_FIELD_NULLABLE[4]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, &(user_value){.bit_field_value = 0x1});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, &(user_value){.bit_field_value = 0x2});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, &(user_value){.bit_field_value = 0x3});

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, &(user_value){.bit_field_value = 0x4});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, &(user_value){.bit_field_value = 0x5});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, &(user_value){.bit_field_value = 0x6});


		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 0, 1, LARGE_UINT_NULLABLE[4]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, &(user_value){.large_uint_value = get_uint256(12)});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, &(user_value){.large_uint_value = get_uint256(13)});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, &(user_value){.large_uint_value = get_uint256(14)});

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, &(user_value){.large_uint_value = get_uint256(15)});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, &(user_value){.large_uint_value = get_uint256(16)});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, &(user_value){.large_uint_value = get_uint256(17)});


		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("", 300);
		data_type_info* array_type_info = &get_fixed_element_count_array_type("", 3, 500, 1, &s4);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 50, &(user_value){.string_value = "Devashree Dvivedi", .string_size = strlen("Devashree Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 50, &(user_value){.string_value = "Rupa Joshi", .string_size = strlen("Rupa Joshi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 50, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.string_value = "Rohan Vipulkumar Dvivedi", .string_size = strlen("Rohan Vipulkumar Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 50, &(user_value){.string_value = "Devashree Joshi", .string_size = strlen("Devashree Joshi")});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 0, NULL_USER_VALUE);
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 0, NULL_USER_VALUE);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, BIT_FIELD_NULLABLE[5]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 50, &(user_value){.bit_field_value = 0x12});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 50, &(user_value){.bit_field_value = 0x16});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.bit_field_value = 0x04});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.bit_field_value = 0x15});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.bit_field_value = 0x15});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.bit_field_value = 0x08});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, UINT_NULLABLE[5]);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 50, &(user_value){.uint_value = 12});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 50, &(user_value){.uint_value = 13});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.uint_value = 14});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.uint_value = 15});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.uint_value = 16});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.uint_value = 17});

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("", 300);
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, &s4);
		finalize_type_info(array_type_info);
		print_type_info(array_type_info);printf("\n");

		test_serde_for_type_info(array_type_info);

		char array[4096];
		initialize_minimal_data_for_type_info(array_type_info, array);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 0, 5, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		expand_container(array_type_info, array, 8, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 0, 50, &(user_value){.string_value = "Rupa Dvivedi", .string_size = strlen("Rupa Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 1, 50, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.string_value = "Devashree Joshi", .string_size = strlen("Devashree Joshi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.string_value = "Vipulkumar Dvivedi", .string_size = strlen("Vipulkumar Dvivedi")});

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		expand_container(array_type_info, array, 2, 3, 200);

		print_data_for_data_type_info(array_type_info, array);printf("\n");

		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 2, 50, &(user_value){.string_value = "Manan Joshi", .string_size = strlen("Manan Joshi")});
		set_user_value_to_containee_in_container_WRAPPER(array_type_info, array, 4, 50, &(user_value){.string_value = "Avyaan Joshi", .string_size = strlen("Avyaan Joshi")});

		print_data_for_data_type_info(array_type_info, array);
		{
			user_value uval;
			get_user_value_for_type_info(&uval, array_type_info, array);
			tuple_def* tpl_d = &(tuple_def){};
			initialize_tuple_def(tpl_d, array_type_info);
			printf("    hashes = %"PRIu64" = %"PRIu64"\n",
				hash_user_value(&uval, array_type_info, FNV_64_TUPLE_HASHER),
				hash_tuple(array, tpl_d, &SELF, FNV_64_TUPLE_HASHER, 1));
		}

		discard_from_container(array_type_info, array, 1, 3);

		print_data_for_data_type_info(array_type_info, array);printf("\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("", 300);
		finalize_type_info(&s4);
		print_type_info(&s4);printf("\n");

		test_serde_for_type_info(&s4);

		char s[4096];
		set_user_value_for_type_info(&s4, s, 0, 300, &(user_value){.string_value = "Rohan Dvivedi", .string_size = strlen("Rohan Dvivedi")});

		print_data_for_data_type_info(&s4, s);printf("\n");

		expand_container(&s4, s, 6, 11, 200);

		print_data_for_data_type_info(&s4, s);printf("\n");

		char* to_add = "Vipulkumar ";
		for(char* c = to_add; (*c) != 0; c++)
			set_user_value_to_containee_in_container_WRAPPER(&s4, s, 6 + (c - to_add), 300, &(user_value){.uint_value = (*c)});

		print_data_for_data_type_info(&s4, s);printf("\n");

		discard_from_container(&s4, s, 11, 5);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));

		set_user_value_for_type_info(&s4, s, 0, 300, EMPTY_USER_VALUE);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));

		int setting_to_null = set_user_value_for_type_info(&s4, s, 0, 300, NULL_USER_VALUE);

		printf("tried setting to null, result = %d\n", setting_to_null);

		print_data_for_data_type_info(&s4, s);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&s4, s));
	}
	printf("\n\n");

	// comapre string and array
	{
		data_type_info str = get_variable_length_string_type("", 300);
		data_type_info arr = get_variable_element_count_array_type("", 300, INT_NULLABLE[4]);

		finalize_type_info(&str);
		finalize_type_info(&arr);

		test_serde_for_type_info(&str);
		test_serde_for_type_info(&arr);

		char string[4096];
		char array[4096];

		set_user_value_for_type_info(&str, string, 0, 300, &(user_value){.string_value = "ABC", .string_size = 3});

		set_user_value_for_type_info(&arr, array, 0, 300, EMPTY_USER_VALUE);
		expand_container(&arr, array, 0, 4, 300);
		set_user_value_to_containee_in_container_WRAPPER(&arr, array, 0, 0, &(user_value){.int_value = 'A'});
		set_user_value_to_containee_in_container_WRAPPER(&arr, array, 1, 0, &(user_value){.int_value = 'B'});
		set_user_value_to_containee_in_container_WRAPPER(&arr, array, 2, 0, &(user_value){.int_value = 'C'});

		user_value string_uval;
		get_user_value_for_type_info(&string_uval, &str, string);
		user_value array_uval;
		get_user_value_for_type_info(&array_uval, &arr, array);

		print_user_value(&string_uval, &str); printf("\n");
		print_user_value(&array_uval, &arr); printf("\n");

		printf("cmp(string, array) = %d\n", compare_user_value(&string_uval, &str, &array_uval, &arr));
	}
	printf("\n\n");

	// set variable length array in variable length array
	{
		data_type_info arr2 = get_variable_element_count_array_type("", 4096, INT_NULLABLE[4]);
		data_type_info arr1 = get_variable_element_count_array_type("", 4096, &arr2);
		data_type_info arr0 = get_variable_element_count_array_type("", 4096, &arr1);
		finalize_type_info(&arr0);
		print_type_info(&arr0);printf("\n");
		tuple_def def;
		initialize_tuple_def(&def, &arr0);

		test_serde_for_type_info(&arr0);

		char data[4096];
		init_tuple(&def, data);
		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));

		expand_element_count_for_element_in_tuple(&def, SELF, data, 0, 4, UINT32_MAX);
		for(int i = 0; i < 4; i++)
		{
			set_element_in_tuple(&def, STATIC_POSITION(i), data, EMPTY_USER_VALUE, UINT32_MAX);

			expand_element_count_for_element_in_tuple(&def, STATIC_POSITION(i), data, 0, 4, UINT32_MAX);
			for(int j = 0; j < 4; j++)
			{
				set_element_in_tuple(&def, STATIC_POSITION(i, j), data, EMPTY_USER_VALUE, UINT32_MAX);

				expand_element_count_for_element_in_tuple(&def, STATIC_POSITION(i, j), data, 0, 4, UINT32_MAX);
				for(int k = 0; k < 4; k++)
				{
					set_element_in_tuple(&def, STATIC_POSITION(i, j, k), data, &(user_value){.int_value = (i * 4 * 4 + j * 4 + k)}, UINT32_MAX);
				}
			}
		}

		printf("\n");
		printf("intiialized : : \n");
		print_tuple(data, &def);
		printf("\n");


		printf("\n");
		printf("setting nested variable sized element to EMPTY_USER_VALUE : : \n");
		set_element_in_tuple(&def, STATIC_POSITION(2, 1), data, EMPTY_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);
		printf("\n");

		printf("\n");
		printf("setting nested variable sized element to NULL_USER_VALUE : : \n");
		set_element_in_tuple(&def, STATIC_POSITION(2, 1), data, NULL_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);
		printf("\n");


		printf("\n");
		printf("discarding : : \n");
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				for(int k = 0; k < 4; k++)
				{
					set_element_in_tuple(&def, STATIC_POSITION(i, j, k), data, &(user_value){.int_value = -1}, UINT32_MAX);
					print_tuple(data, &def);
					printf("\n");

					{
						if(i == 2 && j == 2 && k == 2)
						{
							print_pre_order_ly(&def, data, STATIC_POSITION(2, 1), 5);
							print_pre_order_ly(&def, data, STATIC_POSITION(2, 2), 5);
							print_pre_order_ly(&def, data, STATIC_POSITION(2, 3), 5);
							print_pre_order_ly(&def, data, STATIC_POSITION(2), 5);
							print_pre_order_ly(&def, data, SELF, 5);
						}
					}

				}
				discard_elements_from_element_in_tuple(&def, STATIC_POSITION(i, j), data, 0, 4);
				print_tuple(data, &def);
				printf("\n");
			}
			discard_elements_from_element_in_tuple(&def, STATIC_POSITION(i), data, 0, 4);
			print_tuple(data, &def);
			printf("\n");
		}
		discard_elements_from_element_in_tuple(&def, SELF, data, 0, 4);
		print_tuple(data, &def);
		printf("\n");

		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));

	}
	printf("\n\n");

	{
		data_type_info arr2 = get_fixed_element_count_array_type("", 4, 0, 1, INT_NON_NULLABLE[4]);
		data_type_info arr1 = get_fixed_element_count_array_type("", 4, 0, 1, &arr2);
		data_type_info arr0 = get_fixed_element_count_array_type("", 4, 0, 1, &arr1);
		finalize_type_info(&arr0);
		print_type_info(&arr0);printf("\n");
		tuple_def def;
		initialize_tuple_def(&def, &arr0);

		test_serde_for_type_info(&arr0);

		char data[4096];
		init_tuple(&def, data);
		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));

		for(int i = 0; i < 4; i++)
		{
			set_element_in_tuple(&def, STATIC_POSITION(i), data, EMPTY_USER_VALUE, UINT32_MAX);
			for(int j = 0; j < 4; j++)
			{
				set_element_in_tuple(&def, STATIC_POSITION(i, j), data, EMPTY_USER_VALUE, UINT32_MAX);
				for(int k = 0; k < 4; k++)
				{
					set_element_in_tuple(&def, STATIC_POSITION(i, j, k), data, &(user_value){.int_value = (i * 4 * 4 + j * 4 + k)}, UINT32_MAX);
				}
			}
		}

		printf("\n");
		printf("intiialized : : \n");
		print_tuple(data, &def);
		printf("\n");


		printf("\n");
		printf("setting nested variable sized element to EMPTY_USER_VALUE : : \n");
		set_element_in_tuple(&def, STATIC_POSITION(2, 1), data, EMPTY_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);
		printf("\n");

		printf("\n");
		printf("setting nested variable sized element to NULL_USER_VALUE : : \n");
		set_element_in_tuple(&def, STATIC_POSITION(2, 1), data, NULL_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);
		printf("\n");


		printf("\n");
		printf("discarding : : \n");
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				for(int k = 0; k < 4; k++)
				{
					set_element_in_tuple(&def, STATIC_POSITION(i, j, k), data, &(user_value){.int_value = 0}, UINT32_MAX);
					print_tuple(data, &def);
					printf("\n");
				}
			}
		}

		printf("\n");
		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));

		printf("\nsetting all to NULL and checking if it is minimal\n");
		set_element_in_tuple(&def, STATIC_POSITION(0), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(1), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(2), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(3), data, NULL_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));

		printf("\nsetting all to EMPTY then NULL and checking if it is minimal\n");
		set_element_in_tuple(&def, STATIC_POSITION(0), data, EMPTY_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(1), data, EMPTY_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(2), data, EMPTY_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(3), data, EMPTY_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(0), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(1), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(2), data, NULL_USER_VALUE, UINT32_MAX);
		set_element_in_tuple(&def, STATIC_POSITION(3), data, NULL_USER_VALUE, UINT32_MAX);
		print_tuple(data, &def);printf("    is minimal = %d\n", is_minimal_data_for_type_info(&arr0, data));
	}
	printf("\n\n");

	{
		tuple_size_def tsd;
		initialize_tuple_size_def(&tsd, UINT_NULLABLE[6]);

		print_tuple_size_def(&tsd);printf("\n");

		char serial_tsd[13];
		uint32_t serial_tsd_size = serialize_tuple_size_def(&tsd, serial_tsd);

		tuple_size_def tsd2;
		int res = deserialize_tuple_size_def(&tsd2, serial_tsd, serial_tsd_size);
		if(res)
			print_tuple_size_def(&tsd2);
		else
			printf("failed to deserialize\n");
	}
	printf("\n\n");

	{
		data_type_info s1 = get_fixed_length_string_type("", 10, 0);
		data_type_info s2 = get_fixed_length_string_type("", 12, 1);
		data_type_info s3 = get_variable_length_string_type("", 100);
		data_type_info s4 = get_variable_length_string_type("", 300);

		data_type_info* tuple_type_info = alloca(sizeof_tuple_data_type_info(8));
		initialize_tuple_data_type_info(tuple_type_info, "tuple_type1", 1, 1024, 8);
		tuple_type_info->containees[0].al.type_info = UINT_NON_NULLABLE[3];
		tuple_type_info->containees[1].al.type_info = BIT_FIELD_NON_NULLABLE[5];
		tuple_type_info->containees[2].al.type_info = &s3;
		tuple_type_info->containees[3].al.type_info = &s1;
		tuple_type_info->containees[4].al.type_info = &s2;
		tuple_type_info->containees[5].al.type_info = &s4;
		tuple_type_info->containees[6].al.type_info = FLOAT_double_NULLABLE;
		tuple_type_info->containees[7].al.type_info = BIT_FIELD_NULLABLE[5];
		for(int i = 0; i < 8; i++)
			strcpy(tuple_type_info->containees[i].field_name, "");
		finalize_type_info(tuple_type_info);

		tuple_size_def tsd;
		initialize_tuple_size_def(&tsd, tuple_type_info);

		print_tuple_size_def(&tsd);printf("\n");

		char serial_tsd[13];
		uint32_t serial_tsd_size = serialize_tuple_size_def(&tsd, serial_tsd);

		tuple_size_def tsd2;
		int res = deserialize_tuple_size_def(&tsd2, serial_tsd, serial_tsd_size);
		if(res)
			print_tuple_size_def(&tsd2);
		else
			printf("failed to deserialize\n");
	}
	printf("\n\n");

	{
		data_type_info s4 = get_variable_length_string_type("", 300);
		finalize_type_info(&s4);

		tuple_size_def tsd;
		initialize_tuple_size_def(&tsd, &s4);

		print_tuple_size_def(&tsd);printf("\n");

		char serial_tsd[13];
		uint32_t serial_tsd_size = serialize_tuple_size_def(&tsd, serial_tsd);

		tuple_size_def tsd2;
		int res = deserialize_tuple_size_def(&tsd2, serial_tsd, serial_tsd_size);
		if(res)
			print_tuple_size_def(&tsd2);
		else
			printf("failed to deserialize\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, UINT_NULLABLE[5]);
		finalize_type_info(array_type_info);

		tuple_size_def tsd;
		initialize_tuple_size_def(&tsd, array_type_info);

		print_tuple_size_def(&tsd);printf("\n");

		char serial_tsd[13];
		uint32_t serial_tsd_size = serialize_tuple_size_def(&tsd, serial_tsd);

		tuple_size_def tsd2;
		int res = deserialize_tuple_size_def(&tsd2, serial_tsd, serial_tsd_size);
		if(res)
			print_tuple_size_def(&tsd2);
		else
			printf("failed to deserialize\n");
	}
	printf("\n\n");

	{
		data_type_info* array_type_info = &get_variable_element_count_array_type("", 256, BIT_FIELD_NULLABLE[5]);
		finalize_type_info(array_type_info);

		tuple_size_def tsd;
		initialize_tuple_size_def(&tsd, array_type_info);

		print_tuple_size_def(&tsd);printf("\n");

		char serial_tsd[13];
		uint32_t serial_tsd_size = serialize_tuple_size_def(&tsd, serial_tsd);

		tuple_size_def tsd2;
		int res = deserialize_tuple_size_def(&tsd2, serial_tsd, serial_tsd_size);
		if(res)
			print_tuple_size_def(&tsd2);
		else
			printf("failed to deserialize\n");
	}
	printf("\n\n");

	return 0;
}