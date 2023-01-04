#ifndef platform_low_latency_queue_ipc__h
#define platform_low_latency_queue_ipc__h

#include <aRibeiroPlatform/PlatformQueueIPC.h>
#include <aRibeiroPlatform/PlatformSemaphoreIPC.h>

namespace aRibeiro {

    class PlatformLowLatencyQueueIPC {

        bool blocking_on_read;

        std::string name;

        std::string header_name;
        std::string buffer_name;
        std::string semaphore_name;//mutex
        std::string semaphore_count_name;//real semaphore
        
        void lock(bool from_constructor = false);
        void unlock(bool from_constructor = false);

        void write_buffer(const uint8_t* data, uint32_t size);
        void read_buffer(uint8_t* data, uint32_t size);

        void releaseAll(bool release_semaphore_ipc);
        void onAbort(const char *file, int line, const char *message);

        PlatformMutex shm_mutex;
    public:

        // unblocked if the containing threads have been interrupted...
        PlatformSemaphoreIPC *semaphore_ipc;

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

        PlatformLowLatencyQueueIPC(const char* name = "default",
            uint32_t mode = PlatformQueueIPC_READ | PlatformQueueIPC_WRITE,
            uint32_t queue_size_ = 64, 
            uint32_t buffer_size_ = 1024,
            bool blocking_on_read_ = true );

        bool writeHasEnoughSpace(uint32_t size, bool lock_if_true = false);
        bool writeHasEnoughSpace(const ObjectBuffer &inputBuffer, bool lock_if_true = false);
        bool write(const uint8_t *data, uint32_t size, bool blocking = true, bool ignore_first_lock = false);
        bool write(const ObjectBuffer &inputBuffer, bool blocking = true, bool ignore_first_lock = false);

        bool read(ObjectBuffer *outputBuffer);

        virtual ~PlatformLowLatencyQueueIPC();

        void printStats();

        // only check if this queue is signaled for the current thread... 
        // it may be active in another thread...
        bool isSignaled();

    };

}


#endif