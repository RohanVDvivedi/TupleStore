#include<tuplestore/fast_path_comparators.h>

int fast_path_compare_uint64(const datum* uval1, const datum* uval2)
{
	return compare_numbers(uval1->uint_value, uval2->uint_value);
}

int fast_path_compare_int64(const datum* uval1, const datum* uval2)
{
	return compare_numbers(uval1->int_value, uval2->int_value);
}

int fast_path_compare_float(const datum* uval1, const datum* uval2)
{
	if(isnan(uval1->float_value) && isnan(uval2->float_value))
		return 0;
	else if(isnan(uval1->float_value))
		return 1;
	else if(isnan(uval2->float_value))
		return -1;
	else
		return compare_numbers(uval1->float_value, uval2->float_value);
}

int fast_path_compare_double(const datum* uval1, const datum* uval2)
{
	if(isnan(uval1->double_value) && isnan(uval2->double_value))
		return 0;
	else if(isnan(uval1->double_value))
		return 1;
	else if(isnan(uval2->double_value))
		return -1;
	else
		return compare_numbers(uval1->double_value, uval2->double_value);
}

int fast_path_compare_uint256(const datum* uval1, const datum* uval2)
{
	return compare_uint256(uval1->large_uint_value, uval2->large_uint_value);
}

int fast_path_compare_int256(const datum* uval1, const datum* uval2)
{
	return compare_int256(uval1->large_int_value, uval2->large_int_value);
}

int fast_path_compare_string_binary(const datum* uval1, const datum* uval2)
{
	int cmp = 0;
	uint32_t element_count1 = uval1->string_or_binary_size;
	uint32_t element_count2 = uval2->string_or_binary_size;
	uint32_t element_count = min(element_count1, element_count2);

	cmp = memcmp(uval1->string_or_binary_value, uval2->string_or_binary_value, element_count);
	if(cmp != 0)
		cmp = (cmp > 0) ? 1 : -1;

	if(cmp == 0 && (element_count1 != element_count2))
	{
		if(element_count1 > element_count2)
			cmp = 1;
		else
			cmp = -1;
	}

	return cmp;
}