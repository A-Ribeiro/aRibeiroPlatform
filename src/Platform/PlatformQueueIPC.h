#ifndef platform_queue_ipc__h
#define platform_queue_ipc__h

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/ObjectBuffer.h>

#if defined(OS_TARGET_win)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

    #include <stdio.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <unistd.h>
    #include <signal.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>

#else
#error QueueIPC for this platform is not implemented...
#endif

#include <string>
//#include <stdint.h>

namespace aRibeiro {

    const uint32_t PlatformQueueIPC_READ = 1 << 0;
    const uint32_t PlatformQueueIPC_WRITE = 1 << 1;

    struct PlatformQueueHeader {
        uint32_t subscribers_count;

        uint32_t write_pos;
        uint32_t read_pos;

        uint32_t queue_size;
        uint32_t buffer_size;

        uint32_t capacity;
        uint32_t size;
    };

    struct PlatformBufferHeader {
        uint32_t size;
    };

    class PlatformQueueIPC {

        std::string name;

        std::string header_name;
        std::string buffer_name;
        std::string semaphore_name;
        
        void lock(bool from_constructor = false);
        void unlock(bool from_constructor = false);

        void write_buffer(const uint8_t* data, uint32_t size);
        void read_buffer(uint8_t* data, uint32_t size);

        //private copy constructores, to avoid copy...
        PlatformQueueIPC(const PlatformQueueIPC& v){}
        void operator=(const PlatformQueueIPC& v){}

        void releaseAll();
        void onAbort(const char *file, int line, const char *message);

        PlatformMutex shm_mutex;
    public:

    #if defined(OS_TARGET_win)
        HANDLE queue_semaphore;
        HANDLE queue_header_handle;
        HANDLE queue_buffer_handle;
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        sem_t* queue_semaphore; //semaphore
        int queue_header_handle; //FD
        int queue_buffer_handle; //FD

        int f_lock;

    #endif

        PlatformQueueHeader* queue_header_ptr;// read/write queue header
        uint8_t* queue_buffer_ptr;// readonly or writeonly

        PlatformQueueIPC(const char* name = "default",
            uint32_t mode = PlatformQueueIPC_READ | PlatformQueueIPC_WRITE,
            uint32_t queue_size_ = 64, 
            uint32_t buffer_size_ = 1024 );

        bool writeHasEnoughSpace(uint32_t size, bool lock_if_true = false);
        bool writeHasEnoughSpace(const ObjectBuffer &inputBuffer, bool lock_if_true = false);
        bool write(const uint8_t *data, uint32_t size, bool blocking = true, bool ignore_first_lock = false);
        bool write(const ObjectBuffer &inputBuffer, bool blocking = true, bool ignore_first_lock = false);

        bool readHasElement(bool lock_if_true = false);
        bool read(ObjectBuffer *outputBuffer, bool blocking = true, bool ignore_first_lock = false);

        virtual ~PlatformQueueIPC();

        void printStats();

    };

}


#endif