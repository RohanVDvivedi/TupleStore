#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<alloca.h>

#include<tuple.h>
#include<page_layout.h>

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

tuple_def* get_tuple_definition()
{
	// initialize tuple definition and insert element definitions
	tuple_def* def = get_new_tuple_def("my_table", 16, PAGE_SIZE);

	int res = insert_element_def(def, "col_0", INT, 5, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 0\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_1", UINT, 1, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 1\n");
		exit(-1);
	}

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		res = insert_element_def(def, "col_2", STRING, 15, 0, NULL);
	#else
		res = insert_element_def(def, "var_col_2", VAR_STRING, VAR_STRING_SIZE_SPECIFICER_SIZE, 0, NULL);
	#endif
	if(res == 0)
	{
		printf("failed adding column 2\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_3", FLOAT, 8, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 3\n");
		exit(-1);
	}

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		res = insert_element_def(def, "col_4", STRING, 8, 0, NULL);
	#else
		res = insert_element_def(def, "var_col_4", VAR_STRING, VAR_STRING_SIZE_SPECIFICER_SIZE, 0, NULL);
	#endif
	if(res == 0)
	{
		printf("failed adding column 4\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_5", INT, 1, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 5\n");
		exit(-1);
	}
	res = insert_element_def(def, "col_6", INT, 1, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 6\n");
		exit(-1);
	}
	res = insert_element_def(def, "col_7", INT, 1, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 7\n");
		exit(-1);
	}
	res = insert_element_def(def, "col_8", INT, 1, 0, NULL);
	if(res == 0)
	{
		printf("failed adding column 8\n");
		exit(-1);
	}

	res = insert_element_def(def, "col_3", FLOAT, 4, 0, NULL);
	printf("attempting to insert an element def with the 'col_2' name => %d\n", res);

	res = insert_element_def(def, "col_1", FLOAT, 4, 0, NULL);
	printf("attempting to insert an element def with the 'col_1' name => %d\n", res);

	res = insert_element_def(def, "col_0", FLOAT, 4, 0, NULL);
	printf("attempting to insert an element def with the 'col_4' name => %d\n", res);

	res = finalize_tuple_def(def);
	if(res == 0)
	{
		printf("failed finalizing tuple definition\n");
		exit(-1);
	}

	if(is_empty_tuple_def(def))
	{
		printf("ERROR BUILDING TUPLE DEFINITION\n");
		exit(-1);
	}

	print_tuple_def(def);
	printf("\n\n");

	return def;
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

	set_element_in_tuple(def, column_no++, tuple, &((user_value){.int_value = r->c0}));
	set_element_in_tuple(def, column_no++, tuple, &((user_value){.uint_value = r->c1}));
	set_element_in_tuple(def, column_no++, tuple, (r->c2 == NULL) ? NULL : &((user_value){.data = r->c2, .data_size = strlen(r->c2)}));
	set_element_in_tuple(def, column_no++, tuple, &((user_value){.double_value = r->c3}));
	set_element_in_tuple(def, column_no++, tuple, (r->c4 == NULL) ? NULL : &((user_value){.data = r->c4, .data_size = strlen(r->c4)}));
	
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
								((uint32_t[]){2}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3), 
								def,
								((uint32_t[]){2}),
								((compare_direction[]){DECREASING}),
								1)
			);

	printf("compare(tuple_3 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 3), 
								def,
								((uint32_t[]){2}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((uint32_t[]){2}),
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
								get_element_def_count_tuple_def(def))
			);

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 0), 
								def,
								((uint32_t[]){2}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((uint32_t[]){2}),
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
								get_element_def_count_tuple_def(def))
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((uint32_t[]){2, 3}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((uint32_t[]){2, 3}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((uint32_t[]){1, 0}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((uint32_t[]){1, 0}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 1), 
								def,
								((uint32_t[]){3, 2}),
								get_nth_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2), 
								def,
								((uint32_t[]){3, 2}),
								((compare_direction[]){DECREASING, DECREASING}),
								2)
			);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- UPDATE INPLACE

	printf("updating inplace at tuple_index 1 and element_index 2\n");
	res = set_element_in_tuple_in_place_on_page(page, PAGE_SIZE, def, 1, 2, &((user_value){.data = "Rohan is bad boy", .data_size = strlen("Rohan is bad boy")}));
	printf("This update must fail on variable_sized tuple_def : %d\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// --------------- UPDATE INPLACE

	printf("updating inplace at tuple_index 1 and element_index 2\n");
	res = set_element_in_tuple_in_place_on_page(page, PAGE_SIZE, def, 1, 2, &((user_value){.data = "Rohan is dad", .data_size = strlen("Rohan is dad")}));
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

	// ---------------- DISCARDING TRAILING TOMB STONES

	discard_trailing_tomb_stones_on_page(page, PAGE_SIZE, &(def->size_def));
	printf("Discarding trailing tomb stones : \n\n\n");

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

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

	res = run_page_compaction(temp_page, PAGE_SIZE, &(def->size_def));
	printf("first compaction : %d\n\n", res);
	res = run_page_compaction(temp_page, PAGE_SIZE, &(def->size_def));
	printf("subsequent compaction : %d\n\n", res);

	printf("\nAfter compaction available_size(%u)\n", get_free_space_on_page(temp_page, PAGE_SIZE, &(def->size_def)));
	printf("\nTEMP PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
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
		}
	}

	// ---------------  UPDATE AT ARBITRARY PLACE

	r = &(row){3, 76, "Rohan Dvivedi's, project.", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple_on_page(page, PAGE_SIZE, &(def->size_def), 2, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

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
	delete_tuple_def(def);

	return 0;
}

void test_updates_inserts_inside_tuple(const tuple_def* def, void* tuple)
{
	init_tuple(def, tuple);

	{
		int64_t c0 = 123;

		set_element_in_tuple(def, 0, tuple, &((user_value){.int_value = c0}));

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		c0 = 456;

		set_element_in_tuple(def, 0, tuple, &((user_value){.int_value = c0}));

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, 0, tuple, NULL);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, 0, tuple, NULL);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

#ifndef TEST_FIXED_ARRAY_PAGE_LAYOUT
	set_element_in_tuple(def, 2, tuple, &((user_value){.data = "Rohan", .data_size = strlen("Rohan")}));
	set_element_in_tuple(def, 4, tuple, &((user_value){.data = "Dvivedi", .data_size = strlen("Dvivedi")}));

	printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
	print_tuple(tuple, def);
	printf("\n\n");

	{
		set_element_in_tuple(def, 2, tuple, NULL);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		char* c2 = "Hello";

		set_element_in_tuple(def, 2, tuple, &((user_value){.data = c2, .data_size = strlen(c2)}));

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		c2 = "World";

		set_element_in_tuple(def, 4, tuple, &((user_value){.data = c2, .data_size = strlen(c2)}));

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, 2, tuple, NULL);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");

		set_element_in_tuple(def, 2, tuple, NULL);

		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

  // set data to make tuple exceed max_size, and it should fail
  // below test cases and their calculations assume that column 2 is NULL
	{
		int res = set_element_in_tuple(def, 2, tuple, &((user_value){.data = NULL, .data_size = PAGE_SIZE - get_tuple_size(def, tuple) - get_element_def_by_id(def, 2)->size_specifier_prefix_size + 5}));

		printf("%d = failed to make tuple size cross max_size\n", res);
		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}

	{
		int res = set_element_in_tuple(def, 2, tuple, &((user_value){.data = "Mr. Rohan Vipulkumar Dvivedi", .data_size = strlen("Mr. Rohan Vipulkumar Dvivedi")}));

		printf("%d = success to update a value to column 2\n", res);
		printf("Built tuple : size(%u)\n\t", get_tuple_size(def, tuple));
		print_tuple(tuple, def);
		printf("\n\n");
	}
#endif
}