#ifndef Windows_Pipe_h___
#define Windows_Pipe_h___

#if defined(_WIN32)

#include <aRibeiroCore/common.h>

//#include <aRibeiroCore/aRibeiroCore.h>
//#include <aRibeiroPlatform/aRibeiroPlatform.h>

#include <aRibeiroPlatform/ObjectBuffer.h>
#include <string>

#define INVALID_FD NULL
#define PIPE_READ_FD 0
#define PIPE_WRITE_FD 1

namespace aRibeiro
{

    // STDOUT_FILENO | STDERR_FILENO
    //void SinkStdFD(int fd);

    class WindowsPipe
    {
        bool read_is_blocking;

        bool read_signaled;
        bool write_signaled;

        HANDLE read_event;
        OVERLAPPED overlapped;

        static std::string randomNamedPipeName();

    public:
        union
        {
            int fd[2];
            struct
            {
                HANDLE read_fd;
                HANDLE write_fd;
            };
        };

        WindowsPipe();
        ~WindowsPipe();

        size_t write(const void *buffer, size_t size);

        // returns true if read something...
        // false the other case ...
        // need to call isSignaled if receive false
        bool read(aRibeiro::ObjectBuffer *outputBuffer);

        bool isReadSignaled();

        bool isWriteSignaled();

        //// STDIN_FILENO
        //void aliasReadAs(int fd = STDIN_FILENO);

        //// STDOUT_FILENO | STDERR_FILENO
        //void aliasWriteAs(int fd = STDOUT_FILENO);

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
