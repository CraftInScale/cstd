#include "FileDescriptorReader.h"

#include "../error.h"
#include "../string/String.h"
#include "../string/StringConversion.h"
#include "../log/Logger.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int create_fdreader(FileDescriptorReader& reader, int fd, size_t buf_size)
{
	reader.buf = (uint8_t*)malloc(buf_size);
	if (reader.buf == nullptr)
		return E_ERRNO;

	reader.buf_capacity = buf_size;
	reader.buf_size = 0;
	reader.read_offset = 0;
	reader.fd = fd;

	return E_OK;
}

void destroy_fdreader(FileDescriptorReader& reader)
{
	if (reader.buf != nullptr)
	{
		free(reader.buf);
		reader.buf = nullptr;
	}
}

int fdreader_read_buf(FileDescriptorReader& reader)
{
	ssize_t size = read(reader.fd, reader.buf, reader.buf_capacity);

	if (size == -1)
		return E_ERRNO;
	else if (size == 0)
		return E_EOF;

	reader.buf_size = size;
	reader.read_offset = 0;

	return E_OK;
}

int fdreader_read_buf(FileDescriptorReader& reader, uint8_t* buf, size_t buf_size, size_t& read_size)
{
	read_size = 0;

	int error;
	while (buf_size > 0)
	{
		if (reader.read_offset >= reader.buf_size)
		{
			error = fdreader_read_buf(reader);
			if (error != E_OK)
				return error;
		}

		size_t available_size = reader.buf_size - reader.read_offset;
		size_t copy_size = 0;
		if (available_size < buf_size)
		{
			copy_size = available_size;
			buf_size -= available_size;
			buf += available_size;
		}
		else if (available_size >= buf_size)
		{
			copy_size = buf_size;
			buf_size = 0;
		}

		memcpy(buf, reader.buf + reader.read_offset, copy_size);
		read_size += copy_size;
		reader.read_offset += copy_size;
	}

	return E_OK;
}

int fdreader_read_byte(FileDescriptorReader& reader, uint8_t& byte)
{
	int error;
	if (reader.read_offset >= reader.buf_size)
	{
		error = fdreader_read_buf(reader);
		if (error != E_OK)
			return error;
	}

	byte = reader.buf[reader.read_offset];
	reader.read_offset += 1;

	return E_OK;
}
