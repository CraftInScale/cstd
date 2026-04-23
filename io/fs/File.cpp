#include "File.h"

#include "../../error.h"
#include "../common.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int create_file(File& file, Path& filepath)
{
    file.fd = -1;

    int error;
    E_RET_IF_ERROR_USE(path_get_type(filepath, file.type));
    E_RET_IF_ERROR_USE(path_clone(filepath, file.path));

    return E_OK;
}

void destroy_file(File& file)
{
    destroy_path(file.path);
    if (file.fd > -1)
    {
        close(file.fd);
        file.fd = -1;
    }
}

bool file_exists(File& file)
{
    return file.type != PTYPE_NOT_EXISTS;
}

int copy_file(Path& srcpath, Path& dstpath)
{
    int fd_to, fd_from;
    char buf[FILE_BUF_SIZE];
    ssize_t nread;
    int saved_errno;

    fd_from = open(string_c_str(srcpath.inner), O_RDONLY);
    if (fd_from < 0)
        return E_ERRNO;

    fd_to = open(string_c_str(dstpath.inner), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char* out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } 
        while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = E_ERRNO;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return E_OK;
    }

out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return E_ERRNO;
}

int copy_file_if_not_exists(Path& filepath, Path& resourcepath)
{
    bool exists;
    int error = path_exists(filepath, exists);
    if (error != E_OK)
        return error;

    if (!exists)
    {
        error = path_exists(resourcepath, exists);
        if (error != E_OK)
            return error;

        if (!exists)
            return E_FILE_NOT_FOUND;

        error = copy_file(resourcepath, filepath);
    }

    return error;
}

int file_open(File& file, FileOptions& opts)
{
    int flags = fopts_to_open_flags(opts);

    int fd = open(string_c_str(file.path.inner), fopts_to_open_flags(opts), permissions_to_mode(opts.permissions));
    if (fd == -1)
        return E_ERRNO;

    file.fd = fd;

    return E_OK;
}

int file_close(File& file)
{
    int error = close(file.fd);
    if (error == -1)
        return E_ERRNO;

    file.fd = -1;

    return E_OK;
}

