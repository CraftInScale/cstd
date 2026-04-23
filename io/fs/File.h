#pragma once

#include "Path.h"
#include "FileOptions.h"

struct File
{
	Path path;
	PathType type;
	int fd;
};

int create_file(File& file, Path& filepath);
void destroy_file(File& file);
int copy_file(Path& srcpath, Path& dstpath);
int copy_file_if_not_exists(Path& dstpath, Path& srcpath);

bool file_exists(File& file);
//int file_create_link(File& file);

int file_open(File& file, FileOptions& opts);
int file_close(File& file);