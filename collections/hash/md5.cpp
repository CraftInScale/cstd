#include "md5.h"

#include <strings.h>
#include <string.h>

inline uint32_t rotl(uint32_t n, uint8_t by)
{
	return (n << by) | (n >> (32 - by));
}

void md5(Md5State& result, void* data, size_t data_size)
{
	const uint32_t shifts[] = {
		7,12,17,22,  7,12,17,22,  7,12,17,22,  7,12,17,22,
		5,9,14,20,  5,19,14,20,  5,9,14,20,  5,9,14,20,
		4,11,16,23,  4,11,16,23,  4,11,16,23,  4,11,16,23,
		6,10,15,21,  6,10,15,21,  6,10,15,21,  6,10,15,21
	};

	const uint32_t K[] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	// prepare data
	uint32_t data_fragment[16];
	
	uint8_t* data_ptr = (uint8_t*)data;
	size_t left_size = data_size;

	// init md5
	// 67'45'23'01'ef'cd'ab'89 ' 98'ba'dc'fe'10'32'54'76
	result.A = 0x67'45'23'01;
	result.B = 0xef'cd'ab'89;
	result.C = 0x98'ba'dc'fe;
	result.D = 0x10'32'54'76;

	while (left_size > 0)
	{
		// fill with void* data
		size_t iter_bytes_size = left_size >= 64 ? 64 : left_size;

		memcpy(data_fragment, data_ptr, iter_bytes_size);
		data_ptr += iter_bytes_size;

		// pad with 0s
		if(iter_bytes_size < 64)
			bzero(((uint8_t*)data_fragment) + iter_bytes_size, 64 - iter_bytes_size);

		left_size -= iter_bytes_size;

		// run md5

		Md5State state;
		state.A = result.A;
		state.B = result.B;
		state.C = result.C;
		state.D = result.D;

		for (int i = 0; i < 64; ++i)
		{
			uint32_t f;
			uint32_t g;

			if (i < 16)
			{
				f = (state.B & state.C) | ((~state.B) & state.D);
				g = i;
			}
			else if (i < 32)
			{
				f = (state.D & state.B) | ((~state.D) & state.C);
				g = (5 * i + 1) % 16;
			}
			else if (i < 48)
			{
				f = state.B ^ state.C ^ state.D;
				g = (3 * i + 5) % 16;
			}
			else
			{
				f = state.C ^ (state.B | (~state.D));
				g = (7 * i) % 16;
			}

			f += state.A + K[i] + data_fragment[g];
			
			state.A = state.D;
			state.D = state.C;
			state.C = state.B;
			state.B += rotl(f, shifts[i]);
		}

		result.A += state.A;
		result.B += state.B;
		result.C += state.C;
		result.D += state.D;
	}
}