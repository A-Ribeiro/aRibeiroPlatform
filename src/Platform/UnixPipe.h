#ifndef Unix_Pipe_h___
#define Unix_Pipe_h___

#if !defined(_WIN32)

//#include <aRibeiroCore/aRibeiroCore.h>
//#include <aRibeiroPlatform/aRibeiroPlatform.h>

#include <aRibeiroPlatform/ObjectBuffer.h>

#include <unistd.h> // read(), write(), pipe(), dup2(), getuid()
#include <stdio.h>
#include <string.h> //memset()
#include <errno.h>
#include <fcntl.h> // fcntl()

#define INVALID_FD -1
#define PIPE_READ_FD 0
#define PIPE_WRITE_FD 1

namespace aRibeiro
{

    // STDOUT_FILENO | STDERR_FILENO
    void SinkStdFD(int fd);

    class UnixPipe
    {

        bool read_signaled;
        bool write_signaled;

    public:
        union
        {
            int fd[2];
            struct
            {
                int read_fd;
                int write_fd;
            };
        };

        UnixPipe();
        ~UnixPipe();

        size_t write(const void *buffer, size_t size);

        // returns true if read something...
        // false the other case ...
        // need to call isSignaled if receive false
        bool read(aRibeiro::ObjectBuffer *outputBuffer);

        bool isReadSignaled();

        bool isWriteSignaled();

        // STDIN_FILENO
        void aliasReadAs(int fd = STDIN_FILENO);

        // STDOUT_FILENO | STDERR_FILENO
        void aliasWriteAs(int fd = STDOUT_FILENO);

        bool isReadBlocking();

        bool isWriteBlocking();

        void setReadBlocking(bool v);

        void setWriteBlocking(bool v);

        bool isReadFDClosed();

        bool isWriteFDClosed();

        void closeReadFD();

        void closeWriteFD();

        void close();
    };

}

#endif

#endif
