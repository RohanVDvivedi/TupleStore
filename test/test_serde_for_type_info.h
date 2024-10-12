#include<data_type_info.h>
#include<stdlib.h>
#include<stdio.h>

void test_serde_for_type_info(const data_type_info* dti)
{
	uint32_t bytes_size = get_byte_count_for_serialized_type_info(dti);
	printf("need %"PRIu32" bytes for serializing dti\n", bytes_size);
	char* bytes = malloc(bytes_size);
	uint32_t bytes_used = serialize_type_info(dti, bytes);
	printf("used %"PRIu32" bytes for serializing dti\n", bytes_used);

	if(bytes_size != bytes_used)
	{
		printf("bytes size and bytes used not same\n");
		exit(-1);
	}

	for(uint32_t i = 0; i < bytes_size; i++)
		printf("%03"PRIu8" ", ((unsigned char)bytes[i]));
	printf("\n");

	int allocation_error = 0;
	data_type_info* dti2 = deserialize_type_info(bytes, bytes_used, &allocation_error);
	if(allocation_error)
	{
		printf("encountered allocation error while deserializing dti\n");
		exit(-1);
	}
	if(dti2 == NULL)
	{
		printf("failed deserializing dti\n");
		exit(-1);
	}
	if(!finalize_type_info(dti2))
	{
		printf("could not finalize deserialized dti\n");
		exit(-1);
	}
	if(!are_identical_type_info(dti, dti2))
	{
		printf("deserialization did not produce identical dti\n");
		exit(-1);
	}

	printf("is serialization and deserialization produce identical dtis = %d\n", are_identical_type_info(dti, dti2));

	destroy_non_static_type_info_recursively(dti2);
	printf("\n\n");
}