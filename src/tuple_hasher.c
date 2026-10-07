#include<tuplestore/tuple_hasher.h>

uint64_t tuple_hash_byte(tuple_hasher* th, uint8_t byte)
{
	th->hash_update(&(th->hash), &byte, 1);
	return th->hash;
}

uint64_t tuple_hash_bytes(tuple_hasher* th, const uint8_t* bytes, uint32_t bytes_count)
{
	th->hash_update(&(th->hash), bytes, bytes_count);
	return th->hash;
}

/* SAMPLE IMPLEMENTATION */

#define FNV_64_PRIME ((uint64_t)0x100000001b3ULL)
void fnv_64_update(uint64_t* hash, const uint8_t* bytes, uint32_t bytes_count)
{
	for(uint32_t i = 0; i < bytes_count; i++)
	{
		(*hash) *= FNV_64_PRIME;
		(*hash) = (*hash) ^ (uint64_t)(bytes[i]);
	}
}