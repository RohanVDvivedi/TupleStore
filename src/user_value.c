#include<user_value.h>

#include<tuple_def.h>

#include<inttypes.h>
#include<stdlib.h>

const user_value NULL_USER_VALUE = {.is_NULL = 1};
const user_value ZERO_USER_VALUE = {.is_NULL = 0};

int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}

void print_user_value(const user_value* uval, const element_def* ele_d)
{
	switch(ele_d->type)
	{
		case UINT :
		{
			printf("%"PRIu64, uval->uint_value);
			break;
		}
		case INT :
		{
			printf("%"PRId64, uval->int_value);
			break;
		}
		case FLOAT :
		{
			switch(ele_d->size)
			{
				case 4 :
				{
					printf("%f", uval->float_value);
					break;
				}
				case 8 :
				{
					printf("%lf", uval->double_value);
					break;
				}
			}
			break;
		}
		case STRING :
		case VAR_STRING :
		{
			printf("\"%.*s\"", uval->data_size, ((const char*)(uval->data)));
			break;
		}
		case BLOB :
		case VAR_BLOB :
		{
			printf("BLOB(%"PRIu32")[", uval->data_size);
			for(uint32_t i = 0; i < uval->data_size; i++)
				printf(" 0x%2"PRIx8, ((const uint8_t*)(uval->data))[i]);
			printf("]");
			break;
		}
	}
}