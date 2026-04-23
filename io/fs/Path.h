#pragma once

#include "../../string/String.h"
#include "FileOptions.h"

#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_STR "/"

enum PathType
{
	PTYPE_FILE = 0,
	PTYPE_DIR = 1,
	PTYPE_LINK = 2,
	PTYPE_OTHER = 3,
	PTYPE_NOT_EXISTS = 4
};

struct Path
{
	String inner;
};

int create_path(Path& path, size_t capacity = 0);
int create_path(Path& path, const char* str, size_t str_size = 0, size_t str_len = 0, size_t capacity = 0);
int create_path_from_current_dir(Path& path);
void destroy_path(Path& path);

int path_join(Path& path, const char* relative_path);
int path_get_type(Path& path, PathType& result);
int path_exists(Path& path, bool& result);
int path_clone(Path& path, Path& result_path, bool keep_capacity = false);
int path_parent(Path& path, Path& result_path);
int path_mkdir(Path& path, FilePermissions& permissions, bool recursive);
int path_get_link_path(Path& path, Path& result);
