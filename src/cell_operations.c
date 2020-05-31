#include<cell_operations.h>

#include<string.h>
#include<stdint.h>

int compare_cell_allowed(cell_definition* cell_def_1, cell_definition* cell_def_2)
{
	if(cell_def_1->type == CHAR_STRING && cell_def_2->type == CHAR_STRING)
	{
		return 1;
	}
	else if(cell_def_1->type != CHAR_STRING && cell_def_2->type != CHAR_STRING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int compare_cell_distinct(void* cell_pos_1, cell_definition* cell_def_1, void* cell_pos_2, cell_definition* cell_def_2)
{
	switch(cell_def_1->type)
	{
		case CHAR_STRING :
		{
			switch(cell_def_2->type)
			{
				case CHAR_STRING :
				{
					unsigned int compare_length = CHAR_STRING_max_length;
					return strncmp(cell_pos_1, cell_pos_2, compare_length);
				}
				default :
				{
					return -2;
				}
			}
		}
		case SIGNED_INT :
		{
			int64_t a = get_signed_int_value(cell_pos_1, cell_def_1);
			switch(cell_def_2->type)
			{
				case SIGNED_INT :
				{
					int64_t b = get_signed_int_value(cell_pos_2, cell_def_2);
					return compare_signed_signed(a, b);
				}
				case UNSIGNED_INT :
				{
					uint64_t b = get_unsigned_int_value(cell_pos_2, cell_def_2);
					return compare_signed_unsigned(a, b);
				}
				case FLOATING_NUM :
				{
					double b = get_floating_point_value(cell_pos_2, cell_def_2);
					return compare_float_float(a, b);
				}
				default :
				{
					return -2;
				}
			}
		}
		case UNSIGNED_INT :
		{
			uint64_t a = get_unsigned_int_value(cell_pos_1, cell_def_1);
			switch(cell_def_2->type)
			{
				case SIGNED_INT :
				{
					int64_t b = get_signed_int_value(cell_pos_2, cell_def_2);
					return -compare_signed_unsigned(b, a);
				}
				case UNSIGNED_INT :
				{
					uint64_t b = get_unsigned_int_value(cell_pos_2, cell_def_2);
					return compare_unsigned_unsigned(a, b);
				}
				case FLOATING_NUM :
				{
					double b = get_floating_point_value(cell_pos_2, cell_def_2);
					return compare_float_float(a, b);
				}
				default :
				{
					return -2;
				}
			}
		}
		case FLOATING_NUM :
		{
			double a = get_floating_point_value(cell_pos_1, cell_def_1);
			switch(cell_def_2->type)
			{
				case SIGNED_INT :
				{
					int64_t b = get_signed_int_value(cell_pos_2, cell_def_2);
					return compare_float_float(a, b);
				}
				case UNSIGNED_INT :
				{
					uint64_t b = get_unsigned_int_value(cell_pos_2, cell_def_2);
					return compare_float_float(a, b);
				}
				case FLOATING_NUM :
				{
					double b = get_floating_point_value(cell_pos_2, cell_def_2);
					return compare_float_float(a, b);
				}
				default :
				{
					return -2;
				}
			}
		}
		default :
		{
			return -2;
		}
	}
}