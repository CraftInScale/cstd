#pragma once

#include <stddef.h>
#include <stdint.h>

struct FileDescriptorWriter
{
	int _fd;
	size_t buf_capacity;
	size_t buf_size;
	size_t buf_offset_written;

	uint8_t* buf;// use case of buf is to write in progress-readable way
};

int create_fdwriter(FileDescriptorWriter& writer, int fd, size_t buf_size);
void destroy_fdwriter(FileDescriptorWriter& writer);

/*
Writes data portion with max size of buf.
Write of portions should end with call to fdwriter_flush() in order to save to disk last portion.

@param data is modified, incremented by available space in buf on each call
@param data_size is modified, decremented by size of available space in buf, if 0 then write succeeded
@returns error from write() and fsync()
*/
int fdwriter_write(FileDescriptorWriter& writer, const void*& data, size_t& data_size);
/*
* @param data_size is modified and returns left data size to write (on error)
* use case: writes larger than buffer size
*           that should be fast or at once and no progress reading is needed
* @returns error from write() and fsync()
*/
int fdwriter_write_direct(FileDescriptorWriter& writer, const void* data, size_t data_size);
// @returns error from write() and fsync()
int fdwriter_flush(FileDescriptorWriter& writer);