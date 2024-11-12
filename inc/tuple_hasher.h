#ifndef TUPLE_HASHER_H
#define TUPLE_HASHER_H

typedef struct tuple_hasher tuple_hasher;
struct tuple_hasher
{
	uint64_t hash;

	// update the hash value on an incomming byte
	void hash_update(uint64_t* hash, uint8_t byte);
};

uint64_t tuple_hash_byte(tuple_hasher* th, uint8_t byte);

uint64_t tuple_hash_bytes(tuple_hasher* th, uint8_t* bytes, uint32_t byte_count);

#endif