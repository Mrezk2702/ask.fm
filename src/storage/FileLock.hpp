// src/storage/FileLock.h
#pragma once
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

class FileLock
{
public:
    explicit FileLock(const std::string &filepath)
    {
        fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            std::cerr << "FileLock:open() failed on" << filepath;
            return;
        }

        struct flock lock{};
        lock.l_type = F_WRLCK; // exclusive write lock
        lock.l_whence = SEEK_SET;
        lock.l_start = 0; // start of file
        lock.l_len = 0;   // 0 means entire file

        int flock_result;
        do
        {
            flock_result = fcntl(fd, F_SETLKW, &lock); // F_SETLKW = block and wait until lock is acquired

        } while (flock_result == -1 && errno == EINTR);
        // check if the lock failed or acquiring the lock is interrupted by a signal
        if (flock_result == -1)
        {
            int save_errno = errno;
            switch (save_errno)
            {
            case EBADF:
                std::cerr << "Invalid file descriptor — open() likely failed\n";
                break;
            case EDEADLK:
                std::cerr << "Deadlock detected — two sessions waiting on each other\n";
                break;
            case EINVAL:
                std::cerr << "Invalid flock struct values\n";
                break;
            default:
                std::cerr << "fcntl failed: " << std::strerror(save_errno) << "\n";
                break;
            }
            close(fd);
            fd = -1; // marks lock as not held
        }
    }

    ~FileLock()
    {
        if (fd == -1)
            return;

        struct flock lock{};
        lock.l_type = F_UNLCK; // release lock
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;

        fcntl(fd, F_SETLKW, &lock);
        close(fd);
    }

    // Prevent copying — only one owner of the lock
    FileLock(const FileLock &) = delete;
    FileLock &operator=(const FileLock &) = delete;

    bool is_locked() const { return fd != -1; }

private:
    int fd = -1;
};
