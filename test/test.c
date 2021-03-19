#include<stdio.h>
#include<stdlib.h>

#include<alloca.h>

#include<tuple.h>
#include<page_layout.h>

// comment the below macro to test the SLOTTED_PAGE_LAYOUT
//#define TEST_FIXED_ARRAY_PAGE_LAYOUT

// uncomment the page size that you want to test with
  #define PAGE_SIZE     256
//#define PAGE_SIZE     512
//#define PAGE_SIZE    1024
//#define PAGE_SIZE    4096
//#define PAGE_SIZE    8192
//#define PAGE_SIZE    (((1U<<16) + 1))

char page[PAGE_SIZE] = {};

char temp_page[PAGE_SIZE] = {};

void init_tuple_definition(tuple_def* def)
{
	// initialize tuple definition and insert element definitions
	init_tuple_def(def);

	insert_element_def(def,   INT, 8);

	// a size specifier for a varibale sized string
	insert_element_def(def,   UINT, 1);

	#ifdef TEST_FIXED_ARRAY_PAGE_LAYOUT
		insert_element_def(def, STRING, 15);
	#else
		insert_element_def(def, STRING, VARIABLE_SIZED);
	#endif

	insert_element_def(def,  FLOAT, 8);

	finalize_tuple_def(def);

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
	i8 c0;
	u1 c1;
	char* c2;
	f8 c3;
};

void build_tuple_from_row_struct(const tuple_def* def, void* tuple, const row* r)
{
	int column_no = 0;

	copy_element_to_tuple(def, column_no++, tuple, &(r->c0));
	copy_element_to_tuple(def, column_no++, tuple, &(r->c1));
	copy_element_to_tuple(def, column_no++, tuple,  (r->c2));
	copy_element_to_tuple(def, column_no++, tuple, &(r->c3));

	// output print string
	char print_buffer[PAGE_SIZE];

	sprint_tuple(print_buffer, tuple, def);
	printf("Built tuple : size(%u)\n\t%s\n\n", get_tuple_size(def, tuple), print_buffer);
}

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

	if(!init_page(page, PAGE_SIZE, 3, 7, def))
	{
		printf("ERROR INITIALIZING THE PAGE\n");
		exit(-1);
	}

	// ---------------	INSERT

	r = &(row){-123, 10, "Rohan good", 99.99};

	build_tuple_from_row_struct(def, tuple_cache, r);

	res = insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	printf("Insert : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-12, 12, "Rohan is bad", 51.2};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	printf("Insert : %d\n\n\n", res);

	// ---------------`INSERT

	r = &(row){-12, 13, "Rohan awesome", 20.21};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	printf("Insert : %d\n\n\n", res);

	// ---------------	INSERT

	r = &(row){-53, 2, "ro", 20.21};

	build_tuple_from_row_struct(def, tuple_cache, r);

	insert_tuple(page, PAGE_SIZE, def, tuple_cache);

	printf("Insert : %d\n\n\n", res);

	// ---------------	COMPARE 2 TUPLES

	printf("compare(tuple_0 , tuple_1) = %d\n\n", 
				compare_tuples(	get_nth_tuple(page, PAGE_SIZE, def, 0), 
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def)
			);

	printf("compare(tuple_2 , tuple_1) = %d\n\n", 
				compare_tuples(	get_nth_tuple(page, PAGE_SIZE, def, 2), 
								get_nth_tuple(page, PAGE_SIZE, def, 1), 
								def)
			);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-456, 8, "by Rohan", 65536};

	build_tuple_from_row_struct(def, tuple_cache, r);

	res = update_tuple(page, PAGE_SIZE, def, 1, tuple_cache);

	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------	UPDATE

	r = &(row){-789, 18, "Nam: Rohan Dvivedi", 99.99};

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

	// ---------------	UPDATE

	r = &(row){-123, 17, "Project by Rohan.", 65536};

	build_tuple_from_row_struct(def, tuple_cache, r);

	res = update_tuple(page, PAGE_SIZE, def, 1, tuple_cache);

	printf("Update(1) : %d\n\n\n", res);

	// ---------------	PRINT PAGE
	
	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// --------------- SET REFERENCE PAGE ID

	printf("set reference no %d to %d : %d\n\n", 0, 6, set_reference_page_id(page, 0, 6));
	printf("set reference no %d to %d : %d\n\n", 1, 7, set_reference_page_id(page, 1, 7));
	printf("set reference no %d to %d : %d\n\n", 3, 5, set_reference_page_id(page, 3, 5));
	printf("set reference no %d to %d : %d\n\n", 5, 3, set_reference_page_id(page, 5, 3));
	printf("set reference no %d to %d : %d\n\n", 7, 1, set_reference_page_id(page, 7, 1));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------- INSERT WHEN ABOUT TO BE FULL
/*
	r = &(row){4, -35, 13, "ROHAN IS ART", "test1", "TEST1", 7.12};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_accomodate_tuple_insert(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	r = &(row){5, 36, 12, "Rohan is RD", "test2", "TEST2", 12.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_accomodate_tuple_insert(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	r = &(row){6, -37, 3, "AN", "test3", "TEST3", 7.1996};
	build_tuple_from_row_struct(def, tuple_cache, r);
	printf("Can insert : %d\n\n", can_accomodate_tuple_insert(page, PAGE_SIZE, def, tuple_cache));
	printf("Insert : %d\n\n\n\n", insert_tuple(page, PAGE_SIZE, def, tuple_cache));

	// ---------------- PRINT PAGE

	print_page(page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------	PRINT PAGE IN HEX
	
	print_page_in_hex(page, PAGE_SIZE);

	// ---------------  INSERT TUPLES IN TEMP PAGE BY INDEX RANGE

	init_page(temp_page, PAGE_SIZE, 3, 0, def);
	uint16_t tuples_copied = insert_tuples_from_page(temp_page, PAGE_SIZE, def, page, 1, 4);
	printf("\nTuples copied : %u\n", tuples_copied);
	printf("\nCOPY PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE 2 TUPLES FROM THE PAGE AND COMPACT IT

	delete_tuple(temp_page, PAGE_SIZE, def, 0);
	delete_tuple(temp_page, PAGE_SIZE, def, 2);

	printf("\nBefore compaction available_size(%u)\n", get_free_space_in_page(temp_page, PAGE_SIZE, def));
	printf("\nCOPY PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	run_page_compaction(temp_page, PAGE_SIZE, def);

	printf("\nAfter compaction available_size(%u)\n", get_free_space_in_page(temp_page, PAGE_SIZE, def));
	printf("\nCOPY PAGE :: \n");
	print_page(temp_page, PAGE_SIZE, def);
	printf("\n\n");

	// ---------------  DELETE ALL TUPLES

	uint16_t tuples_to_delete = get_tuple_count(page);
	#define START_WITH (tuples_to_delete/2)

	for(uint16_t i = START_WITH; i < tuples_to_delete; i++)
	{
		uint16_t index = i;
		if(exists_tuple(page, PAGE_SIZE, def, index))
		{
			int is_deleted = delete_tuple(page, PAGE_SIZE, def, index);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
		}
	}

	tuples_to_delete = get_tuple_count(page);
	for(uint16_t i = 0; i < tuples_to_delete; i++)
	{
		uint16_t index = tuples_to_delete - 1 - i;
		if(exists_tuple(page, PAGE_SIZE, def, index))
		{
			int is_deleted = delete_tuple(page, PAGE_SIZE, def, index);
			printf("Delete called for index %u, giving : %d\n", index, is_deleted);
			print_page(page, PAGE_SIZE, def);
			printf("\n\n");
		}
	}

	// ---------------  UPDATE AT ARBITRARY PLACE

	r = &(row){3, 5, 25, "Rohan Dvivedi's, project.", "Roopa", "Vipul", 3.57};
	build_tuple_from_row_struct(def, tuple_cache, r);
	update_tuple(page, PAGE_SIZE, def, 2, tuple_cache);
	print_page(page, PAGE_SIZE, def);

	r = &(row){5, 7, 49, "Rohan's, project : Simple Tuple Storage Model.", "Roopa", "Vipul", 35.7};
	build_tuple_from_row_struct(def, tuple_cache, r);
	update_tuple(page, PAGE_SIZE, def, 0, tuple_cache);
	print_page(page, PAGE_SIZE, def);
*/
	return 0;
}