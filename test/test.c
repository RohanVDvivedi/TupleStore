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

void init_tuple_definition(tuple_def* def)
{
	// initialize tuple definition and insert element definitions
	int res = init_tuple_def(def, "my_table");

	res = insert_element_def(def, "col_0", INT, 8);

	res = insert_element_def(def, "col_1", UINT, 1);

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		res = insert_element_def(def, "col_2", STRING, 15);
	#else
		res = insert_element_def(def, "var_col_2", VAR_STRING, VAR_STRING_SIZE_SPECIFICER_SIZE);
	#endif

	res = insert_element_def(def, "col_3", FLOAT, 8);

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		res = insert_element_def(def, "col_4", STRING, 8);
	#else
		res = insert_element_def(def, "var_col_4", VAR_STRING, VAR_STRING_SIZE_SPECIFICER_SIZE);
	#endif

	res = insert_element_def(def, "col_3", FLOAT, 4);
	printf("attempting to insert an element def with the 'col_2' name => %d\n", res);

	res = insert_element_def(def, "col_1", FLOAT, 4);
	printf("attempting to insert an element def with the 'col_1' name => %d\n", res);

	res = insert_element_def(def, "col_0", FLOAT, 4);
	printf("attempting to insert an element def with the 'col_4' name => %d\n", res);

	finalize_tuple_def(def, PAGE_SIZE);

	if(is_empty_tuple_def(def))
	{
		printf("ERROR BUILDING TUPLE DEFINITION\n");
		exit(-1);
	}

	print_tuple_def(def);
	printf("\n\n");
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

	set_element_in_tuple(def, column_no++, tuple, &((user_value){.int_value = r->c0}));printf("LOL1\n");
	char print_buffer[PAGE_SIZE];
	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

	set_element_in_tuple(def, column_no++, tuple, &((user_value){.uint_value = r->c1}));printf("LOL2\n");
	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

	set_element_in_tuple(def, column_no++, tuple, (r->c2 == NULL) ? NULL : &((user_value){.data = r->c2, .data_size = strlen(r->c2)}));printf("LOL3\n");
	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

	set_element_in_tuple(def, column_no++, tuple, &((user_value){.double_value = r->c3}));printf("LOL4\n");
	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

	set_element_in_tuple(def, column_no++, tuple, (r->c4 == NULL) ? NULL : &((user_value){.data = r->c4, .data_size = strlen(r->c4)}));printf("LOL5\n");
	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);
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
	// allocate size of tuple definition
	tuple_def* def = alloca(size_of_tuple_def(16));

	// initialize tuple definition and insert element definitions
	init_tuple_definition(def);

	// ---------------	DECLARE TEMP variables

	// to build intermediate tuples (only 1 at a time)
	char tuple_cache[PAGE_SIZE];
	// and
	row* r = NULL;
	// and the result from operation
	int res;

	// ---------------  INITIALIZE PAGE

	if(!init_page(page, PAGE_SIZE, sizeof(hdr), def))
	{
		printf("ERROR INITIALIZING THE PAGE\n");
		exit(-1);
	}

	// ---------------	INSERT

	r = &(row){-123, 5, "Rohan is good boy", 99.99};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Insert : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-12, 12, "Rohan is bad", 51.2};
	build_tuple_from_row_struct(def, tuple_cache, r);
	insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Insert : %d\n\n\n", res);

	// ---------------`INSERT

	r = &(row){-12, 12, "Rohan is awesome person", 520.21};
	build_tuple_from_row_struct(def, tuple_cache, r);
	insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Insert : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-53, 17, "Rohan", 20.21};
	build_tuple_from_row_struct(def, tuple_cache, r);
	insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Insert : %d\n\n\n", res);

	// ---------------	COMPARE 2 TUPLES

	printf("compare(tuple_2 , tuple_3) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								((uint32_t[]){2}),
								get_nth_tuple(page, PAGE_SIZE, def, 3), 
								def,
								((uint32_t[]){2}),
								1)
			);

	printf("compare(tuple_3 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 3), 
								def,
								((uint32_t[]){2}),
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								((uint32_t[]){2}),
								1)
			);

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 0), 
								def,
								NULL,
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								NULL,
								def->element_count)
			);

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 0), 
								def,
								((uint32_t[]){2}),
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								((uint32_t[]){2}),
								1)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								NULL,
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								NULL,
								def->element_count)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								((uint32_t[]){2, 3}),
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								((uint32_t[]){2, 3}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								((uint32_t[]){1, 0}),
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								((uint32_t[]){1, 0}),
								2)
			);

	printf("compare(tuple_1 , tuple_2) = %d\n\n", 
				compare_tuples(
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def,
								((uint32_t[]){3, 2}),
								get_nth_tuple(page, PAGE_SIZE, def, 2), 
								def,
								((uint32_t[]){3, 2}),
								2)
			);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-456, 18, "project by Rohan", 65536};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 1, tuple_cache);
	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-789, 9, "Name: Rohan Dvivedi", 99.99};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 3, tuple_cache);
	printf("Update(3) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- DELETE

	res = delete_tuple(page, PAGE_SIZE, def, 1);
	printf("Delete(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- DELETE

	res = delete_tuple(page, PAGE_SIZE, def, 3);
	printf("Delete(3) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-234, 11, "Project by Rohan.", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 1, tuple_cache);
	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- SET PAGE HEADER

	hdr* hdr = get_page_header(page, PAGE_SIZE);
	hdr->data[0] = 0x01234567;
	hdr->data[0] = 0x89abcdef;
	strcpy(hdr->name, "Rohn-pg");

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------- INSERT WHEN ABOUT TO BE FULL

	r = &(row){34, 173, "likes genre horror.", 7.121996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_insert_tuple(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){-35, 13, "Rohan is an artist.", 7.12};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_insert_tuple(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){36, 153, "Rohan is THE Rohan Dvivedi", 12.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_insert_tuple(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){36, 143, "Rohan is just Rohan", 12.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_insert_tuple(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------- INSERT

	r = &(row){35, 49, "Rohan is Dvivedi", 7.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_insert_tuple(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-234, 5, "Project TupleStore", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	UPDATE

	r = &(row){-234, 3, "Project TupleStore", 655.36};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 5, tuple_cache);
	printf("Update(5) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  INSERT TUPLES IN TEMP PAGE BY INDEX RANGE

	init_page(temp_page, PAGE_SIZE, sizeof(hdr), def);
	uint16_t tuples_copied = insert_tuples_from_page(temp_page, PAGE_SIZE, def, page, 1, 4);
	printf("\nTuples copied : %u\n", tuples_copied);
	printf("\nCOPY PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  COMPACT PAGE

	memmove(temp_page, page, PAGE_SIZE);
	delete_tuple(temp_page, PAGE_SIZE, def, 0);
	delete_tuple(temp_page, PAGE_SIZE, def, 2);

	printf("\nBefore compaction available_size(%u)\n", get_free_space(temp_page, PAGE_SIZE, def));
	printf("\nTEMP PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	run_page_compaction(temp_page, PAGE_SIZE, def, 1, 1);

	printf("\nAfter compaction available_size(%u)\n", get_free_space(temp_page, PAGE_SIZE, def));
	printf("\nTEMP PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  SWAP TUPLES

	printf("\nSwap(%u, %u) = %d\n", 2, 3, swap_tuples(temp_page, PAGE_SIZE, def, 2, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 0, 3, swap_tuples(temp_page, PAGE_SIZE, def, 0, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 4, 3, swap_tuples(temp_page, PAGE_SIZE, def, 4, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 1, 3, swap_tuples(temp_page, PAGE_SIZE, def, 1, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	delete_tuple(temp_page, PAGE_SIZE, def, 0);
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 0, 5, swap_tuples(temp_page, PAGE_SIZE, def, 0, 5));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	printf("\nSwap(%u, %u) = %d\n", 0, 3, swap_tuples(temp_page, PAGE_SIZE, def, 0, 3));
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE ALL TUPLES AT ONCE

	delete_all_tuples(temp_page, PAGE_SIZE, def);
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE ALL TUPLES

	printf("page :: \n");
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	uint16_t tuples_to_delete = get_tuple_count(page, PAGE_SIZE, def);
	uint16_t mid = (tuples_to_delete/2);

	for(uint16_t i = mid; i < tuples_to_delete; i++)
	{
		uint16_t index = i;
		printf("l1 index : %u\n", index);
		if(exists_tuple(page, PAGE_SIZE, def, index))
		{
			int is_deleted = delete_tuple(page, PAGE_SIZE, def, index);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
		}
	}

	for(uint16_t i = 0; i < mid; i++)
	{
		uint16_t index = mid - 1 - i;
		printf("l2 index : %u\n", index);
		if(exists_tuple(page, PAGE_SIZE, def, index))
		{
			int is_deleted = delete_tuple(page, PAGE_SIZE, def, index);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
		}
	}

	// ---------------  UPDATE AT ARBITRARY PLACE

	r = &(row){3, 76, "Rohan Dvivedi's, project.", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 2, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	r = &(row){5, 94, "Rohan's, project : Simple Tuple Storage Model.", 35.7};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = update_tuple(page, PAGE_SIZE, def, 0, tuple_cache);
	printf("Update(0) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	// ----------------  INSTEAD TRY INSERT

	r = &(row){3, 76, "Rohan Dvivedi's, project.", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Update(2) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	r = &(row){5, 94, "Rohan's, project : Simple Tuple Storage Model.", 35.7};
	build_tuple_from_row_struct(def, tuple_cache, r);
	res = insert_tuple(page, PAGE_SIZE, def, tuple_cache);
	printf("Update(0) : %d\n\n\n", res);
	print_page(page, PAGE_SIZE, def);

	// -----------------  TESTING UPDATES AND DELETED INSIDE TUPLE

	test_updates_inserts_inside_tuple(def, tuple_cache);

	return 0;
}

void test_updates_inserts_inside_tuple(const tuple_def* def, void* tuple)
{
	// output print string
	char print_buffer[PAGE_SIZE];
	init_tuple(def, tuple);

	{
		int64_t c0 = 123;

		set_element_in_tuple(def, 0, tuple, &((user_value){.int_value = c0}));

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		c0 = 456;

		set_element_in_tuple(def, 0, tuple, &((user_value){.int_value = c0}));

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		set_element_in_tuple(def, 0, tuple, NULL);

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		set_element_in_tuple(def, 0, tuple, NULL);

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);
	}

#ifndef TEST_FIXED_ARRAY_PAGE_LAYOUT
	set_element_in_tuple(def, 2, tuple, &((user_value){.data = "Rohan", .data_size = strlen("Rohan")}));
	set_element_in_tuple(def, 4, tuple, &((user_value){.data = "Dvivedi", .data_size = strlen("Dvivedi")}));

	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

	{
		set_element_in_tuple(def, 2, tuple, NULL);

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		char* c2 = "Hello";

		set_element_in_tuple(def, 2, tuple, &((user_value){.data = c2, .data_size = strlen(c2)}));

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		c2 = "World";

		set_element_in_tuple(def, 4, tuple, &((user_value){.data = c2, .data_size = strlen(c2)}));

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		set_element_in_tuple(def, 2, tuple, NULL);

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);

		set_element_in_tuple(def, 2, tuple, NULL);

		sprint_tuple(print_buffer, tuple, def);
		printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);
	}
#endif
}