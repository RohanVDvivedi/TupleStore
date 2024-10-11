#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<tuple.h>
#include<page_layout.h>

#define VAR_STRING_SIZE_SPECIFICER_SIZE 2

// uncomment the page size that you want to test with
//#define PAGE_SIZE     256
  #define PAGE_SIZE     512
//#define PAGE_SIZE    1024
//#define PAGE_SIZE    4096
//#define PAGE_SIZE    8192
//#define PAGE_SIZE    (((1U<<16) + 1))

char page[PAGE_SIZE] = {};

tuple_def tuple_definition;
data_type_info var_string_type_info;

tuple_def* get_tuple_definition()
{
	var_string_type_info = get_variable_length_string_type("", 1 << (VAR_STRING_SIZE_SPECIFICER_SIZE * 8));

	if(!initialize_tuple_def(&tuple_definition, &var_string_type_info))
	{
		printf("failed finalizing tuple definition\n");
		exit(-1);
	}

	print_tuple_def(&tuple_definition);
	printf("\n\n");

	return &tuple_definition;
}

#define user_Value_for(st_str) (user_value){.string_value = st_str, .string_size = sizeof(st_str) - 1}

int main()
{
	// create tuple_def for the test cases
	tuple_def* def = get_tuple_definition();

	// ---------------	DECLARE TEMP variables

	// to build intermediate tuples (only 1 at a time)
	char tuple_cache[PAGE_SIZE];
	// and the result from operation
	int res;

	// ---------------  INITIALIZE PAGE

	if(!init_page(page, PAGE_SIZE, 13, &(def->size_def)))
	{
		printf("ERROR INITIALIZING THE PAGE\n");
		exit(-1);
	}

	// ---------------	INSERT

	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Rohan Dvivedi"), UINT32_MAX);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	INSERT

	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Rupa Dvivedi"), UINT32_MAX);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------`INSERT

	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Devashree Dvivedi"), UINT32_MAX);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	INSERT

	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Vipulkumar Dvivedi"), UINT32_MAX);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE
	
	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Devashree Joshi"), UINT32_MAX);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE
	
	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Devashree Manan Joshi"), UINT32_MAX);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE
	
	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Rohan Vipulkumar Dvivedi"), UINT32_MAX);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 0, tuple_cache);
	printf("Update(0) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE
	
	init_tuple(def, tuple_cache);
	set_element_in_tuple(def, SELF, tuple_cache, &user_Value_for("Rupa Vipulkumar Dvivedi"), UINT32_MAX);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1, tuple_cache);
	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	return 0;
}