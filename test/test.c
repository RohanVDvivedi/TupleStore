#include<stdio.h>

#include<alloca.h>

#include<tuple.h>
#include<tuple_def.h>

int main()
{
	void* data = alloca(1024);

	tuple_def* def = alloca(sizeof(tuple_def) + (sizeof(element_def) * 6));

	init_tuple_def(def);

	insert_element_def(def, UNSIGNED_INT, 8);
	insert_element_def(def,   SIGNED_INT, 1);
	tuple_mark_key_complete(def);
	insert_element_def(def, UNSIGNED_INT, 1);
	insert_element_def(def,  CHAR_STRING, 32);
	insert_element_def(def,  CHAR_STRING, 32);
	insert_element_def(def, FLOATING_NUM, 8);

	printf("row_defined with size of %u with %u elements, of which %u are keys with %u total key size\n", def->size, def->element_count, def->key_element_count, def->key_size);
	for(int i = 0; i < def->element_count; i++)
	{
		printf("\tstarts at %u, type number is %u, size in bytes = %u\n", def->element_defs[i].offset, def->element_defs[i].type, def->element_defs[i].size);
	}

	void* tpl = data;

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

	print_tuple(tpl, def);

	return 0;
}