#include "UnixPipe.h"

#if !defined(_WIN32)

#include <aRibeiroPlatform/PlatformThread.h>

namespace aRibeiro
{

    // STDOUT_FILENO | STDERR_FILENO
    void SinkStdFD(int fd)
    {
        if (fd == STDOUT_FILENO | fd == STDERR_FILENO)
        {
            int o_fd = open("/dev/null", O_WRONLY);
            if (o_fd != -1)
            {
                dup2(o_fd, fd);
                close(o_fd);
            }
            else
            {
                perror("SinkStdFD STDOUT/STDERR...\n");
            }
        }
        else if (fd == STDIN_FILENO)
        {
            int o_fd = open("/dev/null", O_RDONLY);
            if (o_fd != -1)
            {
                dup2(o_fd, fd);
                close(o_fd);
            }
            else
            {
                perror("SinkStdFD STDIN...\n");
            }
        }
    }

    UnixPipe::UnixPipe()
    {
        read_signaled = false;
        write_signaled = false;
        read_fd = INVALID_FD;
        write_fd = INVALID_FD;
        ARIBEIRO_ABORT(pipe(fd) != 0, "UnixPipe ERROR: %s", strerror(errno));
    }
    UnixPipe::~UnixPipe()
    {
        close();
    }

    size_t UnixPipe::write(const void *buffer, size_t size)
    {
        if (write_fd == INVALID_FD || write_signaled)
            return 0;

        ssize_t written = ::write(write_fd, buffer, size);

        // can have another returns
        if (written == -1)
        {
            if (errno == EAGAIN)
            {
                // non blocking
            }
            else
            {
                // any other error
                printf("[UnixPipe] write ERROR: %s", strerror(errno));
                write_signaled = true;
            }
            return 0;
        }

        return (size_t)written;
    }

    // returns true if read something...
    // false the other case ...
    // need to call isSignaled if receive false
    bool UnixPipe::read(aRibeiro::ObjectBuffer *outputBuffer)
    {
        if (read_fd == INVALID_FD || read_signaled)
            return false;
        if (outputBuffer->size == 0)
            outputBuffer->setSize(64 * 1024); // 64k

        ssize_t received = ::read(read_fd, outputBuffer->data, outputBuffer->size);
        if (received > 0)
        {
            outputBuffer->setSize(received);
            return true;
        }
        outputBuffer->setSize(0);
        if (received == 0)
        {
            // EOF
            closeReadFD();
            return false;
        }

        // can have another returns
        if (received == -1)
        {
            if (errno == EAGAIN)
            {
                // non blocking
                return false;
            }
            else
            {
                // any other error
                printf("[UnixPipe] read ERROR: %s", strerror(errno));
                read_signaled = true;
            }
        }

        return false;
    }

    bool UnixPipe::isReadSignaled()
    {
        return read_fd == INVALID_FD || read_signaled || aRibeiro::PlatformThread::isCurrentThreadInterrupted();
    }

    bool UnixPipe::isWriteSignaled()
    {
        return write_fd == INVALID_FD || write_signaled || aRibeiro::PlatformThread::isCurrentThreadInterrupted();
    }

    // STDIN_FILENO
    void UnixPipe::aliasReadAs(int fd)
    {
        dup2(read_fd, fd);
    }

    // STDOUT_FILENO | STDERR_FILENO
    void UnixPipe::aliasWriteAs(int fd)
    {
        dup2(write_fd, fd);
    }

    bool UnixPipe::isReadBlocking()
    {
        auto flags = fcntl(read_fd, F_GETFL);
        return (flags & O_NONBLOCK) != 0;
    }

    bool UnixPipe::isWriteBlocking()
    {
        auto flags = fcntl(write_fd, F_GETFL);
        return (flags & O_NONBLOCK) != 0;
    }

    void UnixPipe::setReadBlocking(bool v)
    {
        auto flags = fcntl(read_fd, F_GETFL);
        fcntl(read_fd, F_SETFL, (v) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
    }

    void UnixPipe::setWriteBlocking(bool v)
    {
        auto flags = fcntl(write_fd, F_GETFL);
        fcntl(write_fd, F_SETFL, (v) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
    }

    bool UnixPipe::isReadFDClosed()
    {
        return read_fd == INVALID_FD;
    }

    bool UnixPipe::isWriteFDClosed()
    {
        return write_fd == INVALID_FD;
    }

    void UnixPipe::closeReadFD()
    {
        if (read_fd == INVALID_FD)
            return;
        ::close(read_fd);
        read_fd = INVALID_FD;
    }

    void UnixPipe::closeWriteFD()
    {
        if (write_fd == INVALID_FD)
            return;
        ::close(write_fd);
        write_fd = INVALID_FD;
    }

    void UnixPipe::close()
    {
        closeReadFD();
        closeWriteFD();
    }
}

#endif
