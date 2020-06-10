#include<stdio.h>

#include<alloca.h>

#include<tuple.h>
#include<page_context.h>

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
/*
	// this is your single lonely datapage
	void* page = alloca(4096);

	// your tuple
	void* tpl = data;

	// Test copy to cell
	u8 col0 = 5796;
	copy_to_cell(tpl, 0, def, &col0);

	i1 col1 = 25;
	copy_to_cell(tpl, 1, def, &col1);

	u1 col2 = 157;
	copy_to_cell(tpl, 2, def, &col2);

	copy_to_cell(tpl, 3, def, "hello");
	
	copy_to_cell(tpl, 4, def, "Rohan");

	f8 col5 = 5.639;
	copy_to_cell(tpl, 5, def, &col5);

	// test sprint_tuple
	char string[1024];
	sprint_tuple(string, tpl, def);

	printf("tuple : %s", string);


	// test sscan_tuple
	char string1[1024] = "7890674321, -125, 126, Risha, Rujul, 4.5963\n";
	sscan_tuple(string1, tpl, def);
	sprint_tuple(string, tpl, def);

	printf("tuple : %s", string);
*/
	return 0;
}