#ifndef platform_buffer_ipc__h
#define platform_buffer_ipc__h

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

    //const uint32_t PlatformBufferIPC_READ = 1 << 0;
    //const uint32_t PlatformBufferIPC_WRITE = 1 << 1;

    class PlatformBufferIPC {

        std::string name;

        std::string semaphore_name;
        std::string buffer_name;
        

        uint8_t* real_data_ptr;

        bool isFirst;

        void releaseAll();
        void onAbort(const char *file, int line, const char *message);
        PlatformMutex shm_mutex;
        bool force_finish_initialization;

        //private copy constructores, to avoid copy...
        PlatformBufferIPC(const PlatformBufferIPC& v){}
        void operator=(const PlatformBufferIPC& v){}
        
    public:

        void lock(bool from_constructor = false);
        void unlock(bool from_constructor = false);

    #if defined(OS_TARGET_win)
        HANDLE buffer_semaphore;
        HANDLE buffer_handle;
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        sem_t* buffer_semaphore; //semaphore
        int buffer_handle; //FD

        int f_lock;// global file lock

    #endif

        uint8_t* data;
        uint32_t size;

        PlatformBufferIPC(const char* name = "default",
            //uint32_t mode = PlatformBufferIPC_READ | PlatformBufferIPC_WRITE,
            uint32_t buffer_size_ = 1024 );
        
        bool isFirstProcess();
        void finishInitialization();

        virtual ~PlatformBufferIPC();
    };

}


#endif