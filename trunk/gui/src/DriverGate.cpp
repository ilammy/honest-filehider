#include "DriverGate.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

DriverGate::DriverGate(const char *device)
  : device(NULL), fd(-1), open(false)
{
    this->device = new char[strlen(device) + 1];
    strcpy(this->device, device);
}

DriverGate::~DriverGate()
{
    close();
    delete [] device;
}

typename DriverGate::OpenStatus DriverGate::tryOpen()
{
    if (open) {
        return ALREADY_OPEN;
    }
    fd = ::open(device, O_RDWR | O_NONBLOCK);
    if (fd == -1) {
        open = false;
        switch (errno) {
        case EBUSY:  return BUSY;
        case ENOENT: return NOT_FOUND;

        default:     return ANOTHER;
        }
    }
    open = true;
    return OPEN;
}

void DriverGate::close()
{
    if (open) {
        ::close(fd);
        fd = -1;
        open = false;
    }
}

typename DriverGate::Status DriverGate::hide(const char *path, unsigned long long *ino)
{
    if (!open) return NOT_OPEN;

    if (path[0] != '/') {
        return INVALID_FORMAT;
    }
    size_t numBytes;
    numBytes = 1 + snprintf(obuffer, buffer_size, "H %s", path);
    write(fd, obuffer, numBytes);
    read(fd, ibuffer, buffer_size);
    if (gotError()) {
        return parseError();
    }
    if (ino) {
        sscanf(ibuffer, "%lld", ino);
    }
    return OKAY;
}

typename DriverGate::Status DriverGate::unhide(unsigned long long ino)
{
    if (!open) return NOT_OPEN;

    size_t numBytes;
    numBytes = 1 + snprintf(obuffer, buffer_size, "U %lld", ino);
    write(fd, obuffer, numBytes);
    read(fd, ibuffer, buffer_size);
    if (gotError()) {
        return parseError();
    }
    return OKAY;
}

typename DriverGate::Status DriverGate::unhideAll()
{
    if (!open) return NOT_OPEN;

    strcpy(obuffer, "C");
    write(fd, obuffer, 2);
    read(fd, ibuffer, buffer_size);
    if (gotError()) {
        return parseError();
    }
    return OKAY;
}

inline
bool DriverGate::gotError() const
{
    return (ibuffer[0] == 'E');
}

typename DriverGate::Status DriverGate::parseError() const
{
    int code;
    sscanf(obuffer, "E%d", &code);
    if (code < 0) {
        code = -code;
    }
    switch (code) {
    case EINVAL:    return INVALID_FORMAT;
    case ENOENT:    return UNKNOWN_FILE;
    case EPERM:     return MOUNT_POINT;
    case EEXIST:    return ALREADY_HIDDEN;
    case ENOTEMPTY: return HIDDEN_PARENT;
    case EBADF:     return LOST_PARENT;
    case ENOMEM:    return MEMORY_FAULT;

    default:        return UNKNOWN_ERROR;
    }
}
