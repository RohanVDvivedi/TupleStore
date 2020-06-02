#include<stdio.h>

#include<alloca.h>

#include<tuple.h>
#include<row_def.h>

int main()
{
	void* data = alloca(1024);

	row_def* row_d = alloca(sizeof(row_def) + (sizeof(col_def) * 6));

	init_row_def(row_d);

	insert_col_def(row_d, UNSIGNED_INT, 8);
	insert_col_def(row_d,   SIGNED_INT, 1);
	insert_col_def(row_d, UNSIGNED_INT, 1);
	insert_col_def(row_d,  CHAR_STRING, 32);
	insert_col_def(row_d,  CHAR_STRING, 32);
	insert_col_def(row_d, FLOATING_NUM, 8);

	printf("row_defined with size of %u with columns : %u\n", row_d->size_in_bytes, row_d->column_count);
	for(int i = 0; i < row_d->column_count; i++)
	{
		printf("\tstarts at %u, type number is %u, size in bytes = %u\n",
				row_d->col_definitions[i].byte_offset, row_d->col_definitions[i].type, row_d->col_definitions[i].size_in_bytes);
	}

	tuple tpl = data;

	uint64_t col0 = 5796;
	set_cell(row_d, tpl, 0, &col0);

	int8_t col1 = 25;
	set_cell(row_d, tpl, 1, &col1);

	uint8_t col2 = 157;
	set_cell(row_d, tpl, 2, &col2);

	set_cell(row_d, tpl, 3, "hello");
	
	set_cell(row_d, tpl, 4, "Rohan");

	double col5 = 5.639;
	set_cell(row_d, tpl, 5, &col5);

	char* format = "\n\n0 => %llu\n1 => %d\n2 => %u\n3 => %s\n4 => %s\n5 => %lf\n\n";

	printf(format,
		*(get_cell(row_d, tpl, 0).UNSIGNED_INT_8),
		*(get_cell(row_d, tpl, 1).SIGNED_INT_1),
		*(get_cell(row_d, tpl, 2).UNSIGNED_INT_1),
		(get_cell(row_d, tpl, 3).CHAR_STRING),
		(get_cell(row_d, tpl, 4).CHAR_STRING),
		*(get_cell(row_d, tpl, 5).FLOATING_NUM_8));

	return 0;
}