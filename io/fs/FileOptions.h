#pragma once

#include <fcntl.h>

enum Permission : int
{
	PERM_READ,
	PERM_READWRITE,
	PERM_READWRITEEXECUTE,
	PERM_WRITE,
	PERM_WRITEEXECUTE,
	PERM_READEXECUTE,
	PERM_EXECUTE,
	PERM_NONE
};

struct FilePermissions
{
	Permission user, group, other;
};

// e.g. 0777
FilePermissions fperms_from_linux(int mode);

struct FileOptions
{
	bool create, read, write, append, trunc;

	// used for file creation
	FilePermissions permissions;
};

FileOptions create_fopts_create_and_r();
FileOptions create_fopts_create_and_w();
FileOptions create_fopts_create_and_rw();
FileOptions create_fopts_read();
FileOptions create_fopts_write_trunc();
FileOptions create_fopts_write();

int fopts_to_open_flags(FileOptions& opts);
mode_t permissions_to_mode(FilePermissions& permissions);