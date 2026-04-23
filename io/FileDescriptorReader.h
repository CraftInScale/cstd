#pragma once

#include "common.h"

#include <stddef.h>
#include <stdint.h>

struct FileDescriptorReader
{
	int fd;

	uint8_t* buf;
	size_t buf_capacity;// total size of buf space
	size_t buf_size;// valid/usable size of buf

	size_t read_offset;
};

int create_fdreader(FileDescriptorReader& reader, int fd, size_t buf_size);
void destroy_fdreader(FileDescriptorReader& reader);

/*
* @returns E_ERRNO; E_EOF if fd buffer empty
*/
int fdreader_read_buf(FileDescriptorReader& reader);
/*
* @returns E_EOF if fd buffer empty
*/
int fdreader_read_buf(FileDescriptorReader& reader, uint8_t* buf, size_t buf_size, size_t& read_size);
/*
* @returns E_OK, E_ERRNO, E_EOF
*/
int fdreader_read_byte(FileDescriptorReader& reader, uint8_t& byte);