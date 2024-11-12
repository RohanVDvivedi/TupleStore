#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<tuple.h>
#include<page_layout.h>

#include<test_serde_for_type_info.h>

// comment the below macro to test the SLOTTED_PAGE_LAYOUT
//#define TEST_FIXED_ARRAY_PAGE_LAYOUT
#define VAR_STRING_SIZE_SPECIFICER_SIZE 2

// uncomment the page size that you want to test with
//#define PAGE_SIZE     256
  #define PAGE_SIZE     512
//#define PAGE_SIZE    1024
//#define PAGE_SIZE    4096
//#define PAGE_SIZE    8192
//#define PAGE_SIZE    (((1U<<16) + 1))

char page[PAGE_SIZE] = {};

char temp_page[PAGE_SIZE] = {};

tuple_def tuple_definition;
char tuple_type_info_memory[sizeof_tuple_data_type_info(9)];
data_type_info* tuple_type_info = (data_type_info*)tuple_type_info_memory;
data_type_info c2_type_info;
data_type_info c4_type_info;

tuple_def* get_tuple_definition()
{
	initialize_tuple_data_type_info(tuple_type_info, "my_table", 1, PAGE_SIZE, 9);

	strcpy(tuple_type_info->containees[0].field_name, "col_0");
	tuple_type_info->containees[0].type_info = INT_NULLABLE[5];

	strcpy(tuple_type_info->containees[1].field_name, "col_1");
	tuple_type_info->containees[1].type_info = UINT_NULLABLE[1];

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		c2_type_info = get_fixed_length_string_type("", 15, 1);
		strcpy(tuple_type_info->containees[2].field_name, "col_2");
		tuple_type_info->containees[2].type_info = &c2_type_info;
	#else
		c2_type_info = get_variable_length_string_type("", 1 << (VAR_STRING_SIZE_SPECIFICER_SIZE * 8));
		strcpy(tuple_type_info->containees[2].field_name, "var_col_2");
		tuple_type_info->containees[2].type_info = &c2_type_info;
	#endif

	strcpy(tuple_type_info->containees[3].field_name, "col_3");
	tuple_type_info->containees[3].type_info = FLOAT_double_NULLABLE;

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		c4_type_info = get_fixed_length_string_type("", 8, 1);
		strcpy(tuple_type_info->containees[4].field_name, "col_4");
		tuple_type_info->containees[4].type_info = &c4_type_info;
	#else
		c4_type_info = get_variable_length_string_type("", 1 << (VAR_STRING_SIZE_SPECIFICER_SIZE * 8));
		strcpy(tuple_type_info->containees[4].field_name, "var_col_4");
		tuple_type_info->containees[4].type_info = &c4_type_info;
	#endif

	strcpy(tuple_type_info->containees[5].field_name, "col_5");
	tuple_type_info->containees[5].type_info = INT_NULLABLE[1];

	strcpy(tuple_type_info->containees[6].field_name, "col_6");
	tuple_type_info->containees[6].type_info = INT_NULLABLE[1];

	strcpy(tuple_type_info->containees[7].field_name, "col_7");
	tuple_type_info->containees[7].type_info = INT_NULLABLE[1];

	strcpy(tuple_type_info->containees[8].field_name, "col_8");
	tuple_type_info->containees[8].type_info = INT_NULLABLE[1];

	if(!initialize_tuple_def(&tuple_definition, tuple_type_info))
	{
		printf("failed finalizing tuple definition\n");
		exit(-1);
	}

	test_serde_for_type_info(tuple_type_info);

	print_tuple_def(&tuple_definition);
	printf("\n\n");

	return &tuple_definition;
}

// a row like struct for ease in building test tuples
typedef struct row row;
struct row
{
	int64_t c0;
	uint8_t c1;
	char* c2;
	double c3;
	char* c4;
};

void build_tuple_from_row_struct(const tuple_def* def, void* tuple, const row* r)
{
	int column_no = 0;

	init_tuple(def, tuple);

	set_element_in_tuple(def, STATIC_POSITION(column_no++), tuple, &((user_value){.int_value = r->c0}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(column_no++), tuple, &((user_value){.uint_value = r->c1}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(column_no++), tuple, (r->c2 == NULL) ? NULL : &((user_value){.string_value = r->c2, .string_size = strlen(r->c2)}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(column_no++), tuple, &((user_value){.double_value = r->c3}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(column_no++), tuple, (r->c4 == NULL) ? NULL : &((user_value){.string_value = r->c4, .string_size = strlen(r->c4)}), UINT32_MAX);
	
	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
	print_tuple(tuple, def);
	printf("\n\n");
}

// a row like struct for ease in building test tuples
typedef struct hdr hdr;
struct hdr
{
	char name[7];
	int data[2];
};

void test_updates_inserts_inside_tuple(const tuple_def* def, void* tuple);

int main()
{
	// create tuple_def for the test cases
	tuple_def* def = get_tuple_definition();

	// ---------------	DECLARE TEMP variables

	// to build intermediate tuples (only 1 at a time)
	char tuple_cache[PAGE_SIZE];
	// and
	row* r = NULL;
	// and the result from operation
	int res;

	// ---------------  INITIALIZE PAGE

	if(!init_page(page, PAGE_SIZE, sizeof(hdr), &(def->size_def)))
	{
		printf("ERROR INITIALIZING THE PAGE\n");
		exit(-1);
	}

	// ---------------	INSERT

	r = &(row){-123, 5, "Rohan is good boy", 99.99};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-12, 12, "Rohan is bad", 51.2};
	build_tuple_from_row_struct(def, tuple_cache, r);
	append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------`INSERT

	r = &(row){-12, 12, "Rohan is awesome person", 520.21};
	build_tuple_from_row_struct(def, tuple_cache, r);
	append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-53, 17, "Rohan", 20.21};
	build_tuple_from_row_struct(def, tuple_cache, r);
	append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);

	// ---------------	COMPARE 2 TUPLES

	printf("compare(tuple_2 , tuple_3) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								((compare_direction[]){DECREASING}),
								1)
			);

	printf("compare(tuple_3 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								((compare_direction[]){DECREASING}),
								1)
			);

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 0), 
								def,
								NULL,
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								NULL,
								((compare_direction[]){DECREASING, DECREASING, DECREASING, DECREASING, DECREASING}),
								5)
			);

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 0), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((positional_accessor[]){STATIC_POSITION(2)}),
								((compare_direction[]){DECREASING}),
								1)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								NULL,
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								NULL,
								((compare_direction[]){DECREASING, DECREASING, DECREASING, DECREASING, DECREASING}),
								5)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((positional_accessor[]){STATIC_POSITION(2), STATIC_POSITION(3)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((positional_accessor[]){STATIC_POSITION(2), STATIC_POSITION(3)}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((positional_accessor[]){STATIC_POSITION(1), STATIC_POSITION(0)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((positional_accessor[]){STATIC_POSITION(1), STATIC_POSITION(0)}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((positional_accessor[]){STATIC_POSITION(3), STATIC_POSITION(2)}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((positional_accessor[]){STATIC_POSITION(3), STATIC_POSITION(2)}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");
	printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- UPDATE INPLACE

	printf("updating inplace at tuple_index 1 and element_index 2\n");
	res = set_element_in_tuple_in_place_on_page(page, PAGE_SIZE, def, 1, STATIC_POSITION(2), &((user_value){.string_value = "Rohan is bad boy", .string_size = strlen("Rohan is bad boy")}));
	printf("This update must fail on variable_sized tuple_def : %d\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// --------------- UPDATE INPLACE

	printf("updating inplace at tuple_index 1 and element_index 2\n");
	res = set_element_in_tuple_in_place_on_page(page, PAGE_SIZE, def, 1, STATIC_POSITION(2), &((user_value){.string_value = "Rohan is dad", .string_size = strlen("Rohan is dad")}));
	printf("This update must not fail : %d\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){-456, 18, "project by Rohan", 65536};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1, tuple_cache);
	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-789, 9, "Name: Rohan Dvivedi", 99.99};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3, tuple_cache);
	printf("Update(3) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- DELETE

	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1, NULL);
	printf("Delete(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");
	printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- DELETE

	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3, NULL);
	printf("Delete(3) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");
	printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");
	printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-234, 11, "Project by Rohan.", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1, tuple_cache);
	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// --------------- APPEND NULL

	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), NULL);
	printf("Append null : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- SET PAGE HEADER

	hdr* hdr_p = get_page_header(page, PAGE_SIZE);
	hdr_p->data[0] = 0x01234567;
	hdr_p->data[1] = 0x89abcdef;
	strcpy(hdr_p->name, "Rohn-pg");

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------- INSERT WHEN ABOUT TO BE FULL

	r = &(row){34, 173, "likes genre horror.", 7.121996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can append : %d\n\n", can_append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));
	printf("Append : %d\n\n\n\n", append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){-35, 13, "Rohan is an artist.", 7.12};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can append : %d\n\n", can_append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));
	printf("Append : %d\n\n\n\n", append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){36, 153, "Rohan is THE Rohan Dvivedi", 12.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can append : %d\n\n", can_append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));
	printf("Append : %d\n\n\n\n", append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){36, 143, "Rohan is just Rohan", 12.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can append : %d\n\n", can_append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));
	printf("Append : %d\n\n\n\n", append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){35, 49, "Rohan is Dvivedi", 7.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can append : %d\n\n", can_append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));
	printf("Append : %d\n\n\n\n", append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-237, 35, "Rohan is THE Rohan Dvivedi", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){-234, 5, "Project TupleStore", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){-234, 3, "Project TupleStore", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){1, 7, "Rohan is THE Rohan Dvivedi of this project", 7.12};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  INSERT TUPLES IN TEMP PAGE BY INDEX RANGE

	init_page(temp_page, PAGE_SIZE, sizeof(hdr), &(def->size_def));
	uint16_t tuples_copied = 0;
	for(uint32_t i = 1; i <= 4; i++)
	{
		if(exists_tuple_on_page(page, PAGE_SIZE, &(def->size_def), i))
		{
			const void* tuple = get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), i);
			if(append_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), tuple))
				tuples_copied++;
		}
	}
	printf("\nTuples copied : %u\n", tuples_copied);
	printf("\nCOPY PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  COMPACT PAGE

	memmove(temp_page, page, PAGE_SIZE);
	update_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), 0, NULL);
	update_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), 2, NULL);
	discard_trailing_tomb_stones_on_page(temp_page, PAGE_SIZE, &(def->size_def));
	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nBefore compaction available_size(%u)\n", get_free_space_on_page(temp_page, PAGE_SIZE, &(def->size_def)));
	printf("\nTEMP PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	int memory_allocation_error = 0;
	res = run_page_compaction(temp_page, PAGE_SIZE, &(def->size_def), &memory_allocation_error);
	if(memory_allocation_error)
	{
		printf("failed :: to allocate memeory for compacting page\n");
		exit(-1);
	}
	printf("first compaction : %d\n\n", res);
	res = run_page_compaction(temp_page, PAGE_SIZE, &(def->size_def), &memory_allocation_error);
	if(memory_allocation_error)
	{
		printf("failed :: to allocate memeory for compacting page\n");
		exit(-1);
	}
	printf("subsequent compaction : %d\n\n", res);

	printf("\nAfter compaction available_size(%u)\n", get_free_space_on_page(temp_page, PAGE_SIZE, &(def->size_def)));
	printf("\nTEMP PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("showing you output before and after zeroing out free space on temp_page\n");
	printf("\nTEMP PAGE :: (before zeroing out free space)\n");
	print_page(temp_page, PAGE_SIZE, def);
	print_page_in_hex(temp_page, PAGE_SIZE);
	printf("\n\n");

	zero_out_free_space_on_page(temp_page, PAGE_SIZE, &(def->size_def));

	printf("\nTEMP PAGE :: (after zeroing out free space)\n");
	print_page(temp_page, PAGE_SIZE, def);
	print_page_in_hex(temp_page, PAGE_SIZE);
	printf("\n\n");

	// ---------------  SWAP TUPLES

	printf("\nSwap(%u, %u) = %d\n", 2, 3, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 2, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 0, 3, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 0, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 4, 3, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 4, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 1, 3, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 1, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	res = update_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), 0, NULL);
	printf("Update(0) : %d\n\n\n", res);
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(temp_page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

  // ---------------  SWAP TUPLES

	printf("\nSwap(%u, %u) = %d\n", 0, 5, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 0, 5));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 0, 3, swap_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def), 0, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// --------------- DISCARD TUPLES

	printf("\nDiscard(%u) = %d\n", 3, discard_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nDiscard(%u) = %d\n", 3, discard_tuple_on_page(temp_page, PAGE_SIZE, &(def->size_def), 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE ALL TUPLES AT ONCE

	discard_all_tuples_on_page(temp_page, PAGE_SIZE, &(def->size_def));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE ALL TUPLES

	printf("page :: \n");
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	uint16_t tuples_to_delete = get_tuple_count_on_page(page, PAGE_SIZE, &(def->size_def));
	uint16_t mid = (tuples_to_delete/2);

	for(uint16_t i = mid; i < tuples_to_delete; i++)
	{
		uint16_t index = i;
		printf("l1 index : %u\n", index);
		if(exists_tuple_on_page(page, PAGE_SIZE, &(def->size_def), index))
		{
			int is_deleted = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), index, NULL);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
			printf("Discarding trailing tomb stones : \n\n\n");

			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
			printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));
		}
	}

	for(uint16_t i = 0; i < mid; i++)
	{
		uint16_t index = mid - 1 - i;
		printf("l2 index : %u\n", index);
		if(exists_tuple_on_page(page, PAGE_SIZE, &(def->size_def), index))
		{
			int is_deleted = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), index, NULL);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
			printf("Discarding trailing tomb stones : \n\n\n");
			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
			printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));
		}
	}

	// ---------------  UPDATE AT ARBITRARY PLACE

	r = &(row){3, 76, "Rohan Dvivedi's, project.", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);
	printf("trailing tomb stones count = %"PRIu32"\n\n", get_trailing_tomb_stones_count_on_page(page, PAGE_SIZE, &(def->size_def)));

	r = &(row){5, 94, "Rohan's, project : Simple Tuple Storage Model.", 35.7};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 0, tuple_cache);
	printf("Update(0) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	// ----------------  INSTEAD TRY INSERT

	r = &(row){3, 76, "Rohan Dvivedi's, project.", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("Append : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	r = &(row){5, 94, "Rohan's, project : Simple Tuple Storage Model.", 35.7};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = append_tuple_on_page(page, PAGE_SIZE, &(def->size_def), tuple_cache);
	printf("insert : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	// -----------------  TESTING UPDATES AND DELETED INSIDE TUPLE

	test_updates_inserts_inside_tuple(def, tuple_cache);

	// delete tuple_def

	return 0;
}

void test_updates_inserts_inside_tuple(const tuple_def* def, void* tuple)
{
	init_tuple(def, tuple);

	{
		int64_t c0 = 123;

		set_element_in_tuple(def, STATIC_POSITION(0), tuple, &((user_value){.int_value = c0}), UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		c0 = 456;

		set_element_in_tuple(def, STATIC_POSITION(0), tuple, &((user_value){.int_value = c0}), UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, STATIC_POSITION(0), tuple, NULL, UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, STATIC_POSITION(0), tuple, NULL, UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

#ifndef TEST_FIXED_ARRAY_PAGE_LAYOUT
	set_element_in_tuple(def, STATIC_POSITION(2), tuple, &((user_value){.string_value = "Rohan", .string_size = strlen("Rohan")}), UINT32_MAX);
	set_element_in_tuple(def, STATIC_POSITION(4), tuple, &((user_value){.string_value = "Dvivedi", .string_size = strlen("Dvivedi")}), UINT32_MAX);

	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
	print_tuple(tuple, def);
	printf("\n\n");

	{
		set_element_in_tuple(def, STATIC_POSITION(2), tuple, NULL, UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		char* c2 = "Hello";

		set_element_in_tuple(def, STATIC_POSITION(2), tuple, &((user_value){.string_value = c2, .string_size = strlen(c2)}), UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		c2 = "World";

		set_element_in_tuple(def, STATIC_POSITION(4), tuple, &((user_value){.string_value = c2, .string_size = strlen(c2)}), UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, STATIC_POSITION(2), tuple, NULL, UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, STATIC_POSITION(2), tuple, NULL, UINT32_MAX);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

  // set data to make tuple exceed max_size, and it should fail
  // below test cases and their calculations assume that column 2 is NULL
	{
		char test_string[4096];
		for(int i = 0; i < 4096; i++)
				test_string[i] = 'A';
		int res = set_element_in_tuple(def, STATIC_POSITION(2), tuple, &((user_value){.string_value = test_string, .string_size = PAGE_SIZE - get_tuple_size(def, tuple) - VAR_STRING_SIZE_SPECIFICER_SIZE + 5}), UINT32_MAX);

		printf("%d = failed to make tuple size cross max_size\n", res);
		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

	{
		int res = set_element_in_tuple(def, STATIC_POSITION(2), tuple, &((user_value){.string_value = "Mr. Rohan Vipulkumar Dvivedi", .string_size = strlen("Mr. Rohan Vipulkumar Dvivedi")}), UINT32_MAX);

		printf("%d = success to update a value to column 2\n", res);
		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}
#endif
}