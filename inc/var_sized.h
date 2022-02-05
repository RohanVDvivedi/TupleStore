#ifndef VAR_SIZED_H
#define VAR_SIZED_H

#include<stdint.h>

typedef struct var_string8 var_string8;
typedef struct var_string8 var_blob8;
struct var_string8
{
	uint8_t size;
	char[] string;
};

typedef struct var_string16 var_string16;
typedef struct var_string16 var_blob16;
struct var_string16
{
	uint16_t size;
	char[] string;
};

typedef struct var_string32 var_string32;
typedef struct var_string32 var_blob32;
struct var_string32
{
	uint32_t size;
	char[] string;
};

#endif