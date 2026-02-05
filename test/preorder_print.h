#include<stdio.h>
#include<stdlib.h>

#include<tuplestore/tuple.h>
#include<tuplestore/tuple_def.h>
#include<tuplestore/datum.h>

void print_pre_order_ly(const tuple_def* tpl_d, const void* tupl, const positional_accessor base_position, uint32_t max_relative_depth)
{
	uint32_t max_absolute_depth = base_position.positions_length + max_relative_depth;
	positional_accessor absolute_position = {.positions = malloc(sizeof(uint32_t) * max_absolute_depth), .positions_length = 0};

	append_positions(&absolute_position, base_position);

	while(1)
	{
		datum uval;
		int valid = get_value_from_element_from_tuple(&uval, tpl_d, absolute_position, tupl);
		const data_type_info* dti = get_type_info_for_element_from_tuple_def(tpl_d, absolute_position);

		if(!valid)
		{
			if((absolute_position.positions_length >= base_position.positions_length + 2) && point_to_next_uncle_position(&absolute_position))
				continue;
			else
				break;
		}

		// set any one of the following after analyzing the current element/type
		int found_result = 0;
		int skip_all_remaining_siblings = 0;
		int skip_all_children = 0;

		// analyze dti and datum
		printf(" = > ");
		print_datum(&uval, dti);
		printf("\n");

		skip_all_children = (dti->type == STRING);

		if(found_result) // you found what you wanted, then just break out
			break;
		else if(skip_all_remaining_siblings) // just processed the candidate and want to skip all its siblings
		{
			if((absolute_position.positions_length >= base_position.positions_length + 2) && point_to_next_uncle_position(&absolute_position))
				continue;
			else
				break;
		}
		else if(skip_all_children) // just processed the candidate and want to skip all its children
		{
			if(absolute_position.positions_length > base_position.positions_length)
			{
				point_to_next_sibling_position(&absolute_position);
				continue;
			}
			else
				break;
		}

		// default way to go next
		if((absolute_position.positions_length < max_absolute_depth) && is_container_type_info(dti) && !is_datum_NULL(&uval))
		{
			point_to_first_child_position(&absolute_position);
			continue;
		}
		else
		{
			if(absolute_position.positions_length > base_position.positions_length)
			{
				point_to_next_sibling_position(&absolute_position);
				continue;
			}
			else
				break;
		}
	}

	free(absolute_position.positions);
}