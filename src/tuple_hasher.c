#include<tuple_hasher.h>

uint64_t tuple_hash_byte(tuple_hasher* th, uint8_t byte)
{
	th->hash_update(&(th->hash), byte);
	return th->hash;
}

uint64_t tuple_hash_bytes(tuple_hasher* th, uint8_t* bytes, uint32_t byte_count)
{
	for(uint32_t i = 0; i < byte_count; i++)
		th->hash_update(&(th->hash), bytes[i]);
	return th->hash;
}