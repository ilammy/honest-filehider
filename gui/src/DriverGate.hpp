#ifndef DRIVER_GATE_H__
#define DRIVER_GATE_H__

class DriverGate {
public:
    enum OpenStatus {
        OPEN,
        ALREADY_OPEN,
        NOT_FOUND,
        BUSY,
        ANOTHER
    };

    enum Status {
        NOT_OPEN = -9,
        UNKNOWN_ERROR = -1,
        OKAY,
        INVALID_FORMAT,
        UNKNOWN_FILE,
        MOUNT_POINT,
        ALREADY_HIDDEN,
        HIDDEN_PARENT,
        LOST_PARENT,
        MEMORY_FAULT
    };
public:
    DriverGate(const char *device);
    ~DriverGate();

    bool isOpen() const { return open; }
    OpenStatus tryOpen();
    void close();

    void setDevice(const char *device);

    Status hide(const char *path, unsigned long long *ino);
    Status unhide(unsigned long long ino);
    Status unhideAll();

private:
    static const unsigned buffer_size = 512;
    char obuffer[buffer_size];
    char ibuffer[buffer_size];

    char *device;
    int fd;
    bool open;

    bool gotError() const;
    Status parseError() const;
};

#endif // DRIVER_GATE_H__
