#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

#include <cstddef>
#include <cstdint>

void MurmurHash3_x64_128(const void* key, size_t len, uint32_t seed, uint64_t* out);

#endif