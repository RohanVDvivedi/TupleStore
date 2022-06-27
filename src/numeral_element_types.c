#include<numeral_element_types.h>

int is_numeral_type_element_def(const element_def* ele_d)
{
	switch(ele_d->type)
	{
		case UINT :
		case INT :
		case FLOAT :
			return 1;
		default :
			return 0;
	}
}