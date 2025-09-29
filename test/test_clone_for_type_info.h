#include<tuplestore/data_type_info.h>
#include<stdlib.h>
#include<stdio.h>

void test_clone_for_type_info(const data_type_info* dti)
{
	int allocation_error = 0;

	data_type_info* cloned_dti = clone_type_info_recursively(dti, &allocation_error, NULL, NULL);

	if(allocation_error)
	{
		printf("encountered allocation error while cloning dti\n");
		exit(-1);
	}
	if(cloned_dti == NULL)
	{
		printf("failed cloning dti\n");
		exit(-1);
	}
	if(!finalize_type_info(cloned_dti))
	{
		printf("could not finalize cloned dti\n");
		exit(-1);
	}
	if(!are_identical_type_info(dti, cloned_dti))
	{
		printf("cloning did not produce identical dti\n");
		exit(-1);
	}

	printf("is cloning produce identical dtis = %d\n", are_identical_type_info(dti, cloned_dti));

	destroy_type_info_recursively(cloned_dti, NULL);
	printf("\n\n");
}