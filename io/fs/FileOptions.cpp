#include "FileOptions.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

static int mode_to_perm(int mode)
{
    switch (mode & 0x7)
    {
    case 1: return PERM_EXECUTE;
    case 2: return PERM_WRITE;
    case 3: return PERM_WRITEEXECUTE;
    case 4: return PERM_READ;
    case 5: return PERM_READEXECUTE;
    case 6: return PERM_READWRITE;
    case 7: return PERM_READWRITEEXECUTE;
    }

    return PERM_NONE;
}

FilePermissions fperms_from_linux(int mode)
{
    FilePermissions perms{ 
        (Permission)mode_to_perm((mode >> 6) & 0x7),
        (Permission)mode_to_perm((mode >> 3) & 0x7),
        (Permission)mode_to_perm(mode & 0x7)
    };

    // rwx bits
    // user group other

    return perms;
}

FileOptions create_fopts_create_and_r()
{
    return FileOptions{
        true, true, false, false, false,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

FileOptions create_fopts_create_and_w()
{
    return FileOptions{
        true, false, true, false, false,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

FileOptions create_fopts_create_and_rw()
{
    return FileOptions{
        true, true, true, false, false,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

FileOptions create_fopts_read()
{
    return FileOptions{
        false, true, false, false, false,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

FileOptions create_fopts_write_trunc()
{
    return FileOptions{
        false, false, true, false, true,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

FileOptions create_fopts_write()
{
    return FileOptions{
        false, false, true, false, false,
        FilePermissions { PERM_READWRITE, PERM_READWRITE, PERM_NONE }
    };
}

int fopts_to_open_flags(FileOptions& opts)
{
    int flags = 0;

    if (opts.create)
        flags |= O_CREAT;

    if (opts.write && opts.trunc)
        flags |= O_TRUNC;

    if (opts.write && opts.append)
        flags |= O_APPEND;

    if (opts.read && opts.write)
        flags |= O_RDWR;
    else if (opts.read)
        flags |= O_RDONLY;
    else if (opts.write)
        flags |= O_WRONLY;

    return flags;
}

mode_t permissions_to_mode(FilePermissions& permissions)
{
    mode_t flags = 0;

    switch (permissions.user)
    {
    case PERM_READ: flags |= S_IRUSR; break;
    case PERM_READWRITE: flags |= S_IRUSR | S_IWUSR; break;
    case PERM_READWRITEEXECUTE: flags |= S_IRUSR | S_IWUSR | S_IXUSR; break;
    case PERM_WRITE: flags |= S_IWUSR; break;
    case PERM_WRITEEXECUTE: flags |= S_IWUSR | S_IXUSR; break;
    case PERM_READEXECUTE: flags |= S_IRUSR | S_IXUSR; break;
    case PERM_EXECUTE: flags |= S_IXUSR; break;
    }

    switch (permissions.group)
    {
    case PERM_READ: flags |= S_IRGRP; break;
    case PERM_READWRITE: flags |= S_IRGRP | S_IWGRP; break;
    case PERM_READWRITEEXECUTE: flags |= S_IRGRP | S_IWGRP | S_IXGRP; break;
    case PERM_WRITE: flags |= S_IWGRP; break;
    case PERM_WRITEEXECUTE: flags |= S_IWGRP | S_IXGRP; break;
    case PERM_READEXECUTE: flags |= S_IRGRP | S_IXGRP; break;
    case PERM_EXECUTE: flags |= S_IXGRP; break;
    }

    switch (permissions.other)
    {
    case PERM_READ: flags |= S_IROTH; break;
    case PERM_READWRITE: flags |= S_IROTH | S_IWOTH; break;
    case PERM_READWRITEEXECUTE: flags |= S_IROTH | S_IWOTH | S_IXOTH; break;
    case PERM_WRITE: flags |= S_IWOTH; break;
    case PERM_WRITEEXECUTE: flags |= S_IWOTH | S_IXOTH; break;
    case PERM_READEXECUTE: flags |= S_IROTH | S_IXOTH; break;
    case PERM_EXECUTE: flags |= S_IXOTH; break;
    }

    return flags;
}