#include "FileDescriptorWriter.h"

#include "../error.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int create_fdwriter(FileDescriptorWriter& writer, int fd, size_t buf_size)
{
	writer._fd = fd;
	writer.buf_capacity = buf_size;
	writer.buf_size = 0;
	writer.buf_offset_written = 0;
	writer.buf = nullptr;

	if (fd <= -1)
		return E_FILE_DESCRIPTOR_NOT_OPEN;

	if (writer.buf_capacity == 0)
		writer.buf_capacity = 1;

	writer.buf = (uint8_t*)malloc(writer.buf_capacity);

	if (writer.buf == nullptr)
		return E_ERRNO;

	return E_OK;
}

void destroy_fdwriter(FileDescriptorWriter& writer)
{
	free(writer.buf);
}

int fdwriter_write(FileDescriptorWriter& writer, const void*& data, size_t& data_size)
{
	size_t left_space = writer.buf_capacity - writer.buf_size;
	size_t write_size = data_size > left_space ? left_space : data_size;

	memcpy(writer.buf + writer.buf_size, data, write_size);

	writer.buf_size += write_size;

	// write buffer if full
	if (writer.buf_size >= writer.buf_capacity)
	{
		int error = fdwriter_flush(writer);
		if (error != E_OK)
		{
			// undo changes on failure
			writer.buf_size -= write_size;
			return E_ERRNO;
		}
	}

	data_size -= write_size;
	data = (void*)(((uint8_t*)data) + write_size);

	return E_OK;
}

int fdwriter_write_direct(FileDescriptorWriter& writer, const void* data, size_t data_size)
{
	ssize_t size;
	while (data_size > 0)
	{
		size = write(writer._fd, data, data_size);
		if (size == -1)
			return E_ERRNO;

		data_size -= size;
		data = (void*)(((uint8_t*)data) + size);
	}

	int error = fsync(writer._fd);
	if (error == -1)
		return E_ERRNO;

	return E_OK;
}

/*int fdwriter_write_direct_chunked(FileDescriptorWriter& writer, void* data, size_t& data_size)
{
	size_t data_size1 = data_size;

	ssize_t size;
	size_t write_size;
	do
	{
		write_size = FILE_BUF_SIZE;
		if (data_size < write_size)
			write_size = data_size;

		size = write(writer._fd, data, write_size);
		if (size == -1)
			return E_ERRNO;

		data_size -= size;
		data = (void*)(((uint8_t*)data) + size);
	}
	while (data_size > 0);

	int error = fsync(writer._fd);
	if (error == -1)
		return E_ERRNO;

	data_size = data_size1;

	return E_OK;
}*/

int fdwriter_flush(FileDescriptorWriter& writer)
{
	size_t left_size = writer.buf_size - writer.buf_offset_written;

	while (left_size > 0)
	{
		ssize_t size = write(writer._fd, writer.buf + writer.buf_offset_written, writer.buf_size - writer.buf_offset_written);
		if (size == -1)
		{
			return E_ERRNO;
		}

		int error = fsync(writer._fd);
		if (error == -1)
			return E_ERRNO;

		writer.buf_offset_written += size;
		left_size -= size;
	}

	if (writer.buf_offset_written == writer.buf_size)
	{
		writer.buf_offset_written = 0;
		writer.buf_size = 0;
	}

	return E_OK;
}


