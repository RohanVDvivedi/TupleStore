#ifndef TUPLE_HASHER_H
#define TUPLE_HASHER_H

#include<stdint.h>

typedef struct tuple_hasher tuple_hasher;
struct tuple_hasher
{
	uint64_t hash;

	// update the hash value on an incomming byte, do not use index, position information of the byte in the buffer or size of the whole buffer in the hash function
	void (*hash_update)(uint64_t* hash, const uint8_t* bytes, uint32_t bytes_count);
};

uint64_t tuple_hash_byte(tuple_hasher* th, uint8_t byte);

uint64_t tuple_hash_bytes(tuple_hasher* th, const uint8_t* bytes, uint32_t bytes_count);

/* SAMPLE IMPLEMENTATION */

void fnv_64_update(uint64_t* hash, const uint8_t* bytes, uint32_t bytes_count);
#define FNV_64_TUPLE_HASHER (&(tuple_hasher){.hash = 0xcbf29ce484222325ULL, .hash_update = fnv_64_update})

#endif