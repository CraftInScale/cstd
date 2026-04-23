#pragma once

#include "../../string/String.h"
#include "../common.h"

#include <stddef.h>
#include <stdint.h>

struct FileReader
{
	size_t buf_capacity;// total size of buffer
	size_t buf_size;// usable size of buffer
	
	// in buff, for line reading or read_buf(buf, buf_size)
	size_t _read_offset;
	int _fd;
	bool _reached_eof;

	uint8_t* buf;
};

int create_freader(FileReader& reader, int fd, size_t buf_size);
void destroy_freader(FileReader& reader);

int freader_read_buf(FileReader& reader);
int freader_read_byte(FileReader& reader, uint8_t* byte);
int freader_read_buf(FileReader& reader, uint8_t* buf, size_t buf_size, size_t& read_size);
/*
* @param result must be created earlier, data is concatenated to it
*/
int freader_read_line(FileReader& reader, String& result);
/*
* @param result creates new string and appends data, or destroys string if error returned except EINTR
*/
int freader_read_to_string(int fd, String& result);

