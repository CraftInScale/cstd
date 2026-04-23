#include "Path.h"

#include "../../error.h"
#include "../../string/Utf8Util.h"

#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int create_path(Path& path, size_t capacity)
{
	return create_string(path.inner, capacity);
}

int create_path(Path& path, const char* str, size_t str_size, size_t str_len, size_t capacity)
{
	return create_string(path.inner, str, str_size, str_len, capacity);
}

int create_path_from_current_dir(Path& path)
{
	char cwd[PATH_MAX];

	if (getcwd(cwd, PATH_MAX) == nullptr)
		return E_ERRNO;

	return create_string(path.inner, cwd);
}

void destroy_path(Path& path)
{
	destroy_string(path.inner);
}

int path_join(Path& path, const char* relative_path)
{
	int error;

	Utf8StrCount count;
	E_RET_IF_ERROR_USE(u8strcount(relative_path, count));

	StringIter iter;
	init_stringiter_backward(iter, relative_path, count.size, count.length);

	uint32_t ch;
	E_RET_IF_ERROR_USE(stringiter_prev(iter, ch));

	E_RET_IF_ERROR_USE(string_concat(path.inner, FILE_SEPARATOR_STR, 1, 1));

	if (ch != FILE_SEPARATOR)
	{
		error = string_concat(path.inner, relative_path, count.size, count.length);
	}
	else
	{
		error = string_concat(path.inner, relative_path, count.size-1, count.length-1);
	}

	if (error != E_OK)
	{
		path.inner.size -= 1;
		path.inner.length -= 1;
		path.inner.data[path.inner.size] = '\0';
	}

	return error;
}

int path_get_type(Path& path, PathType& result)
{
	struct stat buf;
	int err = lstat(string_c_str(path.inner), &buf);

	if (err != 0)
	{
		if (errno != ENOENT)
		{
			return err;
		}
		else
		{
			result = PTYPE_NOT_EXISTS;
		}
	}

	else if (S_ISREG(buf.st_mode))
	{
		result = PTYPE_FILE;
	}

	else if (S_ISDIR(buf.st_mode))
	{
		result = PTYPE_DIR;
	}

	else if (S_ISLNK(buf.st_mode))
	{
		result = PTYPE_LINK;
	}
	else
	{
		result = PTYPE_OTHER;
	}

	return E_OK;
}

int path_exists(Path& path, bool& result)
{
	PathType type_result;
	int error;
	E_RET_IF_ERROR_USE(path_get_type(path, type_result));

	result = type_result != PTYPE_NOT_EXISTS;

	return E_OK;
}

int path_clone(Path& path, Path& result_path, bool keep_capacity)
{
	return string_clone(path.inner, result_path.inner, keep_capacity);
}

int path_parent(Path& path, Path& result_path)
{
	StringIter iter;
	init_stringiter_backward(iter, &path.inner);

	uint32_t ch = 0;
	int error;
	do
	{
		error = stringiter_prev(iter, ch);
		if (error != E_OK && error != E_NULL)
			return error;
	}
	while (ch != FILE_SEPARATOR && ch != 0);

	if (ch == 0)
		return E_DS_EMPTY;

	return create_string(result_path.inner, string_c_str(path.inner), iter.size_off, iter.len_off);
}

int path_mkdir(Path& path, FilePermissions& permissions, bool recursive)
{
	mode_t mode = permissions_to_mode(permissions);

	int error;
	if (!recursive)
	{
		error = mkdir(string_c_str(path.inner), mode);
		if (error == -1)
		{
			if(errno != EEXIST)
				return E_ERRNO;
		}
	}
	else
	{
		StringIter iter;
		init_stringiter_forward(iter, &path.inner);

		// skip first slash
		uint32_t ch;
		error = stringiter_next(iter, ch);
		if (error != E_OK && error != E_NULL)
		{
			return error;
		}

		size_t prev_size, prev_len;
		Path subpath;
		while (ch != 0)
		{
			prev_size = iter.size_off;
			prev_len = iter.len_off;
			error = stringiter_next(iter, ch);
			if (error != E_OK && error != E_NULL)
				return error;

			if (ch == FILE_SEPARATOR || (iter.len_off == path.inner.length && ch != 0))
			{
				Path new_path;
				if (iter.len_off < path.inner.length)
				{
					E_RET_IF_ERROR_USE(create_path(new_path, string_c_str(path.inner), prev_size, prev_len));
				}
				else
				{
					E_RET_IF_ERROR_USE(create_path(new_path, string_c_str(path.inner), iter.size_off, iter.len_off));
				}

				error = mkdir(string_c_str(new_path.inner), mode);
				if (error == -1)
				{
					if (errno != EEXIST)
					{
						destroy_path(new_path);
						return E_ERRNO;
					}
				}

				destroy_path(new_path);
			}
		}
	}

	return E_OK;
}

int path_get_link_path(Path& path, Path& result)
{
	int error = create_path(result, 256);
	if (error != E_OK)
		return error;

	ssize_t size = readlink(string_c_str(path.inner), (char*)result.inner.data, 255);
	if (size == -1)
	{
		destroy_path(result);
		return E_ERRNO;
	}
	else if (size == 255)
	{
		E_RET_IF_ERROR_USE2(string_reserve(result.inner, PATH_MAX - 255), destroy_path(result));
		size = readlink(string_c_str(path.inner), (char*)result.inner.data, PATH_MAX);
		if (size == -1)
		{
			destroy_path(result);
			return E_ERRNO;
		}
	}

	Utf8StrCount count;
	Utf8StrCount limit{ (size_t)size, 0 };
	error = u8strcount((char*)result.inner.data, count, limit);
	if (error != E_OK)
	{
		destroy_path(result);
		return error;
	}

	String& s = result.inner;
	s.size += size;
	s.data[s.size - 1] = 0;
	s.length = count.length;

	return E_OK;
}
