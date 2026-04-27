#include "FileReader.h"

#include "../../error.h"
#include "../../string/Utf8Util.h"
#include "../../string/StringConversion.h"
#include "../../log/Logger.h"

//#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int create_freader(FileReader& reader, int fd, size_t buf_size)
{
    reader._fd = fd;
    reader.buf_capacity = buf_size;
    reader.buf_size = 0;
    //reader._buf_null_byte = null_byte;
    reader._read_offset = 0;
    reader._reached_eof = false;
    reader.buf = nullptr;

    if (fd <= -1)
        return E_FILE_DESCRIPTOR_NOT_OPEN;

    if (reader.buf_capacity == 0)
        reader.buf_capacity = 1;

    /*if (null_byte && (reader.buf_size + 1llu) == 0)
        return E_SIZE_OVERFLOW;*/

    reader.buf = (uint8_t*)malloc(reader.buf_capacity);// malloc( + (reader._buf_null_byte ? 1 : 0))

    if (reader.buf == nullptr)
        return E_ERRNO;

    return E_OK;
}

void destroy_freader(FileReader& reader)
{
    free(reader.buf);
}

int freader_read_buf(FileReader& reader)
{
    if (reader._reached_eof)
        return E_EOF;

    ssize_t ret = read(reader._fd, reader.buf, reader.buf_capacity);
    if (ret == 0)
    {
        reader._reached_eof = true;
        return E_EOF;
    }

    if (ret == -1)
    {
        return E_ERRNO;
    }

    reader.buf_size = ret;
    reader._read_offset = 0;

    return E_OK;
}

int freader_read_byte(FileReader& reader, uint8_t* byte)
{
    if (reader._reached_eof)
    {
        *byte = 0;
        return E_EOF;
    }

    int error;

    if (reader.buf_size == 0)
    {
        // just initialized

        E_RET_IF_ERROR_USE(freader_read_buf(reader));
    }

    if (reader._read_offset >= reader.buf_size)
    {
        E_RET_IF_ERROR_USE(freader_read_buf(reader));
    }

    *byte = *(reader.buf + reader._read_offset);
    reader._read_offset += 1;

    return E_OK;
}

int freader_read_buf(FileReader& reader, uint8_t* buf, size_t buf_size, size_t& read_size)
{
    read_size = 0;

    if (reader._reached_eof)
        return E_EOF;

    int error;
    size_t off;
    while (buf_size > 0)
    {
        if (reader._read_offset == reader.buf_size)
        {
            if (reader._reached_eof)// reader.buf_alloc_size < reader.buf_size
            {
                //reader._reached_eof = true;
                return E_EOF;
            }
            else
            {
                error = freader_read_buf(reader);
                if (error != E_OK)
                {
                    return error;// EOF or ERRNO
                }
            }
        }

        size_t left_to_read = reader.buf_size - reader._read_offset;
        if (left_to_read > buf_size)
        {
            memcpy(buf, reader.buf + reader._read_offset, buf_size);
            reader._read_offset += buf_size;
            read_size += buf_size;
            return E_OK;
        }
        else
        {
            memcpy(buf, reader.buf + reader._read_offset, left_to_read);
            reader._read_offset += left_to_read;
            read_size += left_to_read;
            buf_size -= left_to_read;
            buf += left_to_read;
        }
    }

    return E_OK;
}

int freader_read_line(FileReader& reader, String& result)
{
    if (reader._reached_eof)
        return E_EOF;

    int error;

    if (reader.buf_size == 0)
    {
        // just initialized

        E_RET_IF_ERROR_USE(freader_read_buf(reader));
    }

loop_fr:
    size_t off = reader._read_offset;
    size_t str_size = 0;

    uint8_t* src = reader.buf + reader._read_offset;
    uint8_t* src_end = reader.buf + reader.buf_size;
    bool newline_char = false;
    while (src < src_end)
    {
        reader._read_offset += 1;

        if (*src == '\n' || *src == '\r')
        {
            if (*src == '\r' && src+1 < src_end && *(src+1) == '\n')
            {
                reader._read_offset += 1;
            }
            newline_char = true;
            break;
        }

        src += 1;
        str_size += 1;
    }

    if (!newline_char)
    {
        // copy
        //reader.buf[reader.buf_alloc_size] = 0;
        if (str_size > 0)
        {
            E_RET_IF_ERROR_USE(string_concat(result, (char*)(reader.buf + off), str_size));
        }

        if (reader.buf_size >= reader.buf_capacity)
        {
            E_RET_IF_ERROR_USE(freader_read_buf(reader));

            goto loop_fr;
        }
        else
        {
            reader._reached_eof = true;
            return E_EOF;
        }
    }
    else
    {
        if (str_size > 0)
        {
            error = string_concat(result, (char*)(reader.buf + off), str_size);

            if (error != E_OK)
            {
                return error;
            }
        }
    }

    return E_OK;
}

int freader_read_to_string(int fd, String& result)
{
    off_t cur_off = lseek(fd, 0, SEEK_CUR);
    if (cur_off == -1)
        return E_ERRNO;

    // get file size
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1)
        return E_ERRNO;

    size -= cur_off;

    // reverse effect of lseek
    if (lseek(fd, cur_off, SEEK_SET) == -1)
        return E_ERRNO;

    int error = create_string(result, size_t(size) + 1llu);
    if (error != E_OK)
        return error;

    size_t size1 = size;

    size_t off = 0;
    ssize_t read_size;
    while (size > 0)
    {
        read_size = read(fd, result.data + off, (size_t)size);
        if (read_size == -1)
        {
            if (errno != EINTR)
            {
                destroy_string(result);
                return E_ERRNO;
            }
            else read_size = 0;
        }
        
        size -= read_size;
        off += read_size;
    }

    size_t prev_size = result.size;

    result.size = size1 + 1llu;
    result.data[size1] = 0;

    Utf8StrCount count;
    Utf8StrCount limit{ result.size, 0 };
    error = u8strcount(string_c_str(result), count, limit);
    if (error != E_OK)
    {
        result.size = prev_size;
        return error;
    }

    result.length = count.length;

    return E_OK;
}
