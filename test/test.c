#include<stdio.h>

#include<alloca.h>

#include<tuple.h>
#include<page_context.h>

#define PAGE_SIZE 4096

// output print string
char string[1024];

int main()
{
	// allocate size of tuple definition
	tuple_def* def = alloca(sizeof(tuple_def) + (sizeof(element_def) * 6));

	// initialize tuple definition and insert element definitions
	init_tuple_def(def);
	insert_element_def(def, UNSIGNED_INT, 8);
	insert_element_def(def,   SIGNED_INT, 1);
	tuple_mark_key_complete(def);
	insert_element_def(def, UNSIGNED_INT, 1);
	insert_element_def(def,  CHAR_STRING, 32);
	insert_element_def(def,  CHAR_STRING, 32);
	insert_element_def(def, FLOATING_NUM, 8);

	print_tuple_def(def);

	// this is your single lonely datapage
	void* page = alloca(PAGE_SIZE);

	// to initialize header inside the page
	init_page_header(page, 5, TUPLE_ARRAY);

	// intialize data access methods for your sample bufferpool/page alocator
	data_access_methods dam = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, PAGE_SIZE, NULL};

	// now you can initialize your page context
	page_context pg_cntxt;
	init_page_context(&pg_cntxt, 15, page, def, &dam);

	// your tuple temp storage
	uint32_t tpl_size = def->size;
	void* tpl = alloca(tpl_size);
	uint16_t insr;

	// Test copy to cell
	u8 col0 = 5796;
	i1 col1 = 25;
	u1 col2 = 157;
	char* col3 = "hello";
	char* col4 = "Rohan";
	f8 col5 = 5.639;
	copy_to_cell(tpl, 0, def, &col0);
	copy_to_cell(tpl, 1, def, &col1);
	copy_to_cell(tpl, 2, def, &col2);
	copy_to_cell(tpl, 3, def, "hello");
	copy_to_cell(tpl, 4, def, "Rohan");
	copy_to_cell(tpl, 5, def, &col5);

	// test sprint_tuple
	insr = append_tuples(&pg_cntxt, tpl, 1);
	printf("Inserted : %d\n", insr);

	sprint_tuple(string, tpl, def);
	printf("Inserted tuple : %s", string);

	// test sscan_tuple 
	char tuple2[1024] = "7890674321, -125, 126, Risha, Rujul, 4.5963\n";
	sscan_tuple(tuple2, tpl, def);

	// insert tuple
	insr = append_tuples(&pg_cntxt, tpl, 1);
	printf("Inserted : %d\n", insr);

	sprint_tuple(string, tpl, def);
	printf("Inserted tuple : %s", string);

	string[0] = '\0';

	// test sscan_tuple 
	printf("Printing tuples directly from the page : \n");
	sprint_tuple(string, get_tuple(&pg_cntxt, 0), def);
	printf("tuple 0 : %s", string);
	sprint_tuple(string, get_tuple(&pg_cntxt, 1), def);
	printf("tuple 1 : %s", string);

	return 0;
}