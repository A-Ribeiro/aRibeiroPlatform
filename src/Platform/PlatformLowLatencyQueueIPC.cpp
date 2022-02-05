#include "PlatformLowLatencyQueueIPC.h"
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformSleep.h>
#include <aRibeiroPlatform/PlatformPath.h>

namespace aRibeiro {

#if defined(OS_TARGET_win)
    #define BUFFER_HANDLE_NULL NULL

    static std::string GetLastErrorToString() {
        std::string result;

        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);

        // WCHAR TO CONSOLE CP (Code Page)
        if (s) {
            /*
            wchar_t *aux = new wchar_t[lstrlenW(s)+1];
            memset(aux, 0, (lstrlenW(s)+1) * sizeof(wchar_t));

            //MultiByteToWideChar(CP_ACP, 0L, s, lstrlenA(s) + 1, aux, strlen(s));
            MultiByteToWideChar(
                CP_OEMCP //GetConsoleCP()
                , MB_PRECOMPOSED | MB_ERR_INVALID_CHARS , s, lstrlenA(s) + 1, aux, lstrlenA(s));
                result = StringUtil::toString(aux);
            */

            char *aux = new char[lstrlenW(s) + 1];
            memset(aux, 0, (lstrlenW(s) + 1) * sizeof(char));
            WideCharToMultiByte(
                GetConsoleOutputCP(), //GetConsoleCP(),
                WC_COMPOSITECHECK,
                s, lstrlenW(s) + 1,
                aux, lstrlenW(s),
                NULL, NULL
            );
            result = aux;
            delete[] aux;
            LocalFree(s);
        }

        return result;
    }

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
    #define BUFFER_HANDLE_NULL -1
#endif


    void PlatformLowLatencyQueueIPC::lock(bool from_constructor) {
#if defined(OS_TARGET_win)
        // lock semaphore
        ARIBEIRO_ABORT(WaitForSingleObject(queue_semaphore, INFINITE) != WAIT_OBJECT_0, "Error to lock queue semaphore. Error code: %s\n", GetLastErrorToString().c_str());
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        if (from_constructor) {

            ARIBEIRO_ABORT(f_lock != -1, "Trying to lock twice from constructor.\n");

            // file lock ... to solve the dead semaphore reinitialization...
            std::string global_lock_file = PlatformPath::getDocumentsPath( "aribeiro", "lock" ) + PlatformPath::SEPARATOR + this->name + std::string(".llq.f_lock");

            f_lock = open(global_lock_file.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            ARIBEIRO_ABORT(f_lock == -1, "Error to open f_lock. Error code: %s\n", strerror(errno));

            int rc = lockf(f_lock, F_LOCK, 0);
            ARIBEIRO_ABORT(rc == -1, "Error to lock f_lock. Error code: %s\n", strerror(errno));

            
        } else {
            // while semaphore is signaled, try to aquire until block...
            while (sem_wait(queue_semaphore) != 0);
        }
        
#endif
    }

    void PlatformLowLatencyQueueIPC::unlock(bool from_constructor) {
#if defined(OS_TARGET_win)
        // release semaphore
        ARIBEIRO_ABORT(!ReleaseSemaphore(queue_semaphore, 1, NULL), "Error to unlock queue semaphore. Error code: %s\n", GetLastErrorToString().c_str());
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        if (from_constructor) {
            ARIBEIRO_ABORT(f_lock == -1, "Trying to unlock a non initialized lock from constructor.\n");

            int rc = lockf(f_lock, F_ULOCK, 0);
            ARIBEIRO_ABORT(rc == -1, "Error to unlock f_lock. Error code: %s\n", strerror(errno));

            close(f_lock);
            f_lock = -1;
        }
        else {
            sem_post(queue_semaphore);
        }
#endif
    }

    PlatformLowLatencyQueueIPC::PlatformLowLatencyQueueIPC(const char* name,
        uint32_t mode,
        uint32_t queue_size_, 
        uint32_t buffer_size_,
        bool blocking_on_read_) {

        semaphore_ipc = NULL;

        this->blocking_on_read = blocking_on_read_;

        queue_semaphore = NULL;
        queue_header_handle = BUFFER_HANDLE_NULL;
        queue_buffer_handle = BUFFER_HANDLE_NULL;

        this->name = name;

#if defined(OS_TARGET_win)
        header_name = std::string(name) + std::string("_allqh");//aribeiro_ll_queue_header
        buffer_name = std::string(name) + std::string("_allqb");//aribeiro_ll_queue_buffer
        semaphore_name = std::string(name) + std::string("_allqs");//aribeiro_ll_queue_semaphore
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        header_name = std::string("/") + std::string(name) + std::string("_allqh");//aribeiro_ll_queue_header
        buffer_name = std::string("/") + std::string(name) + std::string("_allqb");//aribeiro_ll_queue_buffer
        semaphore_name = std::string("/") + std::string(name) + std::string("_allqs");//aribeiro_ll_queue_semaphore

        f_lock = -1;
#endif

        semaphore_count_name = std::string(name) + std::string("_allqsc");//aribeiro_ll_queue_semaphore_count

#if defined(OS_TARGET_win)
        SECURITY_DESCRIPTOR sd;
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd,TRUE,(PACL)0,FALSE);
        
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = FALSE;

        queue_semaphore = CreateSemaphoreA(
            &sa, // default security attributes
            1, // initial count
            LONG_MAX, // maximum count
            semaphore_name.c_str() // named semaphore
        );
        ARIBEIRO_ABORT(queue_semaphore == 0, "Error to create global semaphore. Error code: %s\n", GetLastErrorToString().c_str() );


#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
/*

        queue_semaphore = sem_open(
            semaphore_name.c_str(),
            O_CREAT,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            1
        );

        ARIBEIRO_ABORT(queue_semaphore == SEM_FAILED, "Error to create global semaphore. Error code: %s\n", strerror(errno));

*/
#endif

        lock(true);

#if defined(OS_TARGET_win)
        // open the header memory section
        queue_header_handle = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PlatformQueueHeader), header_name.c_str() );
        if (queue_header_handle == 0) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the header IPC queue.\n");
        }
        queue_header_ptr = (PlatformQueueHeader*)MapViewOfFile(queue_header_handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(PlatformQueueHeader));
        if (queue_header_ptr == 0) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to map the header IPC buffer.\n");
        }
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        queue_header_handle = shm_open(header_name.c_str(),
            O_CREAT | O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (queue_header_handle == -1) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the header IPC queue. Error code: %s\n", strerror(errno) );
        }

        int rc;

        struct stat _stat;
        rc = fstat(queue_header_handle, &_stat);
        ARIBEIRO_ABORT(rc != 0, "Error to stat the file descriptor. Error code: %s\n", strerror(errno) );
        if (_stat.st_size == 0) {
            rc = ftruncate(queue_header_handle, sizeof(PlatformQueueHeader));
            //fallocate(queue_header_handle, 0, 0, sizeof(PlatformQueueHeader));
            ARIBEIRO_ABORT(rc != 0, "Error to truncate buffer. Error code: %s\n", strerror(errno) );
        }

        queue_header_ptr = (PlatformQueueHeader*)mmap(
            NULL,
            sizeof(PlatformQueueHeader),
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            queue_header_handle,
            0
        );
        if (queue_header_ptr == MAP_FAILED) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to map the header IPC buffer. Error code: %s\n", strerror(errno) );
        }
#endif

        if (queue_header_ptr->subscribers_count > 0) {
            
            printf("[PlatformLowLatencyQueueIPC] Not First Opened - Retrieving Shared Memory Information...\n");
            semaphore_ipc = new PlatformSemaphoreIPC( semaphore_count_name, 0 );

#if !defined(OS_TARGET_win)
            // initialize semaphore
            queue_semaphore = sem_open(
                semaphore_name.c_str(),
                O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                1
            );

            if (queue_semaphore == SEM_FAILED) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to create global semaphore. Error code: %s\n", strerror(errno));
            }

            lock();//lock the created semaphore
#endif

        }
        else {
            printf("[PlatformLowLatencyQueueIPC] First Opened - Creating Shared Memory...\n");

            semaphore_ipc = new PlatformSemaphoreIPC( semaphore_count_name, 0, true );

#if !defined(OS_TARGET_win)
            //truncate semaphore before initialize it
            sem_unlink(semaphore_name.c_str());

            // initialize semaphore
            queue_semaphore = sem_open(
                semaphore_name.c_str(),
                O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                1
            );

            if (queue_semaphore == SEM_FAILED) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to create global semaphore. Error code: %s\n", strerror(errno));
            }

            lock();//lock the created semaphore
#endif

            queue_header_ptr->write_pos = 0;
            queue_header_ptr->read_pos = 0;

            queue_header_ptr->queue_size = queue_size_;
            queue_header_ptr->buffer_size = buffer_size_ + sizeof(PlatformBufferHeader);
            queue_header_ptr->capacity = queue_header_ptr->buffer_size * queue_header_ptr->queue_size;
            queue_header_ptr->size = 0;


        }

#if defined(OS_TARGET_win)
        // open the buffer memory section
        queue_buffer_handle = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, queue_header_ptr->capacity, buffer_name.c_str() );
        if (queue_buffer_handle == 0) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the buffer IPC queue.\n");
        }
        
        if (mode == ( PlatformQueueIPC_READ | PlatformQueueIPC_WRITE) ) {
            queue_buffer_ptr = (uint8_t*)MapViewOfFile(queue_buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, queue_header_ptr->capacity);
            if (queue_buffer_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        } else if (mode == PlatformQueueIPC_READ) {
            queue_buffer_ptr = (uint8_t*)MapViewOfFile(queue_buffer_handle, FILE_MAP_READ, 0, 0, queue_header_ptr->capacity);
            if (queue_buffer_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        } else if (mode == PlatformQueueIPC_WRITE) {
            queue_buffer_ptr = (uint8_t*)MapViewOfFile(queue_buffer_handle, FILE_MAP_WRITE, 0, 0, queue_header_ptr->capacity);
            if (queue_buffer_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        } else {
            unlock(true);
            ARIBEIRO_ABORT(true, "Queue opening mode not specified.\n");
        }
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        queue_buffer_handle = shm_open(buffer_name.c_str(),
            O_CREAT | O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (queue_buffer_handle == -1) {
            unlock();
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the buffer IPC queue. Error code: %s\n", strerror(errno));
        }

        //int rc;
        //struct stat _stat;
        rc = fstat(queue_buffer_handle, &_stat);
        ARIBEIRO_ABORT(rc != 0, "Error to stat the file descriptor. Error code: %s\n", strerror(errno) );
        if (_stat.st_size == 0) {
            rc = ftruncate(queue_buffer_handle, queue_header_ptr->capacity);
            //fallocate(queue_buffer_handle, 0, 0, queue_header_ptr->capacity);
            ARIBEIRO_ABORT(rc != 0, "Error to truncate buffer. Error code: %s\n", strerror(errno) );
        }

        if (mode == (PlatformQueueIPC_READ | PlatformQueueIPC_WRITE)) {
            queue_buffer_ptr = (uint8_t*)mmap(
                NULL,
                queue_header_ptr->capacity,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                queue_buffer_handle,
                0
            );
            if (queue_buffer_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno));
            }
        }
        else if (mode == PlatformQueueIPC_READ) {
            queue_buffer_ptr = (uint8_t*)mmap(
                NULL,
                queue_header_ptr->capacity,
                PROT_READ,
                MAP_SHARED,
                queue_buffer_handle,
                0
            );
            if (queue_buffer_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno));
            }
        }
        else if (mode == PlatformQueueIPC_WRITE) {
            queue_buffer_ptr = (uint8_t*)mmap(
                NULL,
                queue_header_ptr->capacity,
                PROT_WRITE,
                MAP_SHARED,
                queue_buffer_handle,
                0
            );
            if (queue_buffer_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno));
            }
        }
        else {
            unlock();
            unlock(true);
            ARIBEIRO_ABORT(true, "Queue opening mode not specified.\n");
        }

#endif

        queue_header_ptr->subscribers_count++;

#if !defined(OS_TARGET_win)
        unlock();//unlock the created semaphore
#endif
        unlock(true);




        //
        // Open Shared Header
        //
        /*
        if (mode == PlatformQueueIPC_WRITE) {

            printf("[PlatformLowLatencyQueueIPC] Opening write\n");

            queue_header_handle = CreateFileMappingA(
                INVALID_HANDLE_VALUE, NULL, 
                PAGE_READWRITE, 0, 
                buffer_size, 
                header_name.c_str()
            );
            ARIBEIRO_ABORT(queue_header_handle == 0, "Error to create the header IPC queue.\n");
            queue_header_ptr = (PlatformQueueHeader*)MapViewOfFile(queue_header_handle, FILE_MAP_ALL_ACCESS, 0, 0, buffer_size);
            ARIBEIRO_ABORT(queue_header_ptr == 0, "Error to map the header IPC buffer.\n");


            queue_buffer_handle = CreateFileMappingA(
                INVALID_HANDLE_VALUE, NULL, 
                PAGE_READWRITE, 0, 
                buffer_size, 
                buffer_name.c_str()
            );
            ARIBEIRO_ABORT(queue_buffer_handle == 0, "Error to create the buffer IPC queue.\n");
            queue_buffer_ptr = (uint8_t*)MapViewOfFile(queue_buffer_handle, FILE_MAP_WRITE, 0, 0, buffer_size);
            ARIBEIRO_ABORT(queue_buffer_ptr == 0, "Error to map the IPC buffer.\n");

        } else if (mode == PlatformQueueIPC_READ) {

            printf("[PlatformLowLatencyQueueIPC] Opening read\n");

            queue_header_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, header_name.c_str());
            ARIBEIRO_ABORT(queue_header_handle == 0, "Error to create the header IPC queue.\n");
            queue_header_ptr = (PlatformQueueHeader*)MapViewOfFile(queue_header_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            ARIBEIRO_ABORT(queue_header_ptr == 0, "Error to map the header IPC buffer.\n");

            queue_buffer_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, buffer_name.c_str());
            ARIBEIRO_ABORT(queue_buffer_handle == 0, "Error to create the buffer IPC queue.\n");
            queue_buffer_ptr = (uint8_t*)MapViewOfFile(queue_buffer_handle, FILE_MAP_READ, 0, 0, buffer_size);
            ARIBEIRO_ABORT(queue_buffer_ptr == 0, "Error to map the IPC buffer.\n");
        }
        */
        
        printStats();

    }

    void PlatformLowLatencyQueueIPC::write_buffer(const uint8_t* data, uint32_t size) {
        
        ARIBEIRO_ABORT(size > (queue_header_ptr->capacity - queue_header_ptr->size), "Error to write more than the buffer size\n.");

        uint32_t remaining_space = queue_header_ptr->capacity - queue_header_ptr->write_pos;
        if (remaining_space < size) {
            // two memcpy
            memcpy(&queue_buffer_ptr[queue_header_ptr->write_pos], data, remaining_space);
            memcpy(&queue_buffer_ptr[0], &data[remaining_space], size - remaining_space);
        }
        else {
            // one memcpy
            memcpy(&queue_buffer_ptr[queue_header_ptr->write_pos], data, size);
        }
        queue_header_ptr->write_pos = (queue_header_ptr->write_pos + size) % queue_header_ptr->capacity;
        queue_header_ptr->size += size;

        //printf("\n    Queue write:%u total:%u\n", size,queue_header_ptr->size );
    }

    void PlatformLowLatencyQueueIPC::read_buffer(uint8_t* data, uint32_t size) {

        ARIBEIRO_ABORT(size > queue_header_ptr->size, "Error to read more than the buffer size\n.");

        uint32_t remaining_space = queue_header_ptr->capacity - queue_header_ptr->read_pos;
        if (remaining_space < size) {
            // two memcpy
            memcpy(data, &queue_buffer_ptr[queue_header_ptr->read_pos], remaining_space);
            memcpy(&data[remaining_space], &queue_buffer_ptr[0], size - remaining_space);
        }
        else {
            // one memcpy
            memcpy(data, &queue_buffer_ptr[queue_header_ptr->read_pos], size);
        }
        queue_header_ptr->read_pos = (queue_header_ptr->read_pos + size) % queue_header_ptr->capacity;
        queue_header_ptr->size -= size;

        //printf("\n    Queue reader:%u total:%u\n", size,queue_header_ptr->size );
    }

    bool PlatformLowLatencyQueueIPC::writeHasEnoughSpace(uint32_t size, bool lock_if_true){

        uint32_t size_request = size + sizeof(PlatformBufferHeader);

        ARIBEIRO_ABORT(size_request > queue_header_ptr->capacity, "Buffer too big for this queue.\n");

        lock();

        uint32_t remaining_space = queue_header_ptr->capacity - queue_header_ptr->size;

        if (size_request <= remaining_space) {
            if (!lock_if_true)
                unlock();
            return true;
        } else 
            unlock();

        return false;
    }

    bool PlatformLowLatencyQueueIPC::writeHasEnoughSpace(const ObjectBuffer &inputBuffer, bool lock_if_true) {
        return writeHasEnoughSpace(inputBuffer.size, lock_if_true);
    }

    bool PlatformLowLatencyQueueIPC::write(const uint8_t *data, uint32_t size, bool blocking, bool ignore_first_lock) {

        uint32_t size_request = size + sizeof(PlatformBufferHeader);

        ARIBEIRO_ABORT(size_request > queue_header_ptr->capacity, "Buffer too big for this queue.\n");

        if (!ignore_first_lock) {

            lock();
            uint32_t remaining_space = queue_header_ptr->capacity - queue_header_ptr->size;
            while (size_request > remaining_space) {
                unlock();

                if (!blocking || PlatformThread::isCurrentThreadInterrupted())
                    return false;

                PlatformSleep::sleepMillis(1);
                lock();
                remaining_space = queue_header_ptr->capacity - queue_header_ptr->size;
            }
        }

        PlatformBufferHeader bufferHeader;
        bufferHeader.size = size;

        write_buffer((uint8_t*)&bufferHeader, sizeof(PlatformBufferHeader));
        write_buffer(data, size);

        //if (!ignore_first_lock)
        unlock();

        if (blocking_on_read)
            semaphore_ipc->release();

        return true;
    }

    bool PlatformLowLatencyQueueIPC::write(const ObjectBuffer &inputBuffer, bool blocking, bool ignore_first_lock) {
        return write(inputBuffer.data, inputBuffer.size, blocking, ignore_first_lock);
    }

    bool PlatformLowLatencyQueueIPC::read(ObjectBuffer *outputBuffer) {

        
        if (blocking_on_read) 
        {
            if (!semaphore_ipc->blockingAcquire())
                return false;
        }

        lock();

        if (queue_header_ptr->size == 0) {
            unlock();

            printf("ERROR: Trying to read element from an empty queue.\n");
            return false;
        }

        PlatformBufferHeader bufferHeader;
        read_buffer((uint8_t*)&bufferHeader, sizeof(PlatformBufferHeader));
        outputBuffer->setSize(bufferHeader.size);
        read_buffer(outputBuffer->data, outputBuffer->size);

        unlock();
        return true;
    }

    PlatformLowLatencyQueueIPC::~PlatformLowLatencyQueueIPC() {
        #if !defined(OS_TARGET_win)
            lock(true);
            bool is_last_queue = false;
            std::string sem_count_name = std::string("/") + semaphore_count_name;

            if (queue_header_handle != BUFFER_HANDLE_NULL) {
                is_last_queue = (queue_header_ptr->subscribers_count-1) == 0;
            }
        #endif

        lock();

        if (queue_buffer_handle != BUFFER_HANDLE_NULL) {
#if defined(OS_TARGET_win)
            UnmapViewOfFile(queue_buffer_ptr);
            CloseHandle(queue_buffer_handle);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            munmap(queue_buffer_ptr, queue_header_ptr->capacity);
            close(queue_buffer_handle); //close FD
            //shm_unlink(buffer_name.c_str());
#endif
            queue_buffer_handle = BUFFER_HANDLE_NULL;
        }

        if (queue_header_handle != BUFFER_HANDLE_NULL) {

            queue_header_ptr->subscribers_count--;

#if defined(OS_TARGET_win)
            UnmapViewOfFile(queue_header_ptr);
            CloseHandle(queue_header_handle);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            munmap(queue_header_ptr, sizeof(PlatformQueueHeader));
            close(queue_header_handle); //close FD
            //shm_unlink(header_name.c_str());
#endif
            queue_header_handle = BUFFER_HANDLE_NULL;
        }

        if ( semaphore_ipc != NULL ){

        #if !defined(OS_TARGET_win)
            sem_count_name = semaphore_ipc->name;
        #endif

            delete semaphore_ipc;
            semaphore_ipc = NULL;
        }

        unlock();

        

        if (queue_semaphore != NULL) {
#if defined(OS_TARGET_win)
            CloseHandle(queue_semaphore);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            sem_close(queue_semaphore);
            //sem_unlink(semaphore_name.c_str());
#endif
            
            queue_semaphore = NULL;
        }

        #if !defined(OS_TARGET_win)
            // unlink all resources
            if (is_last_queue) {
                printf("[linux] ... last low latency queue, unlink resources ...\n");
                shm_unlink(buffer_name.c_str());
                shm_unlink(header_name.c_str());
                sem_unlink(semaphore_name.c_str());
                sem_unlink(sem_count_name.c_str());

                //unlink the lock_f
                std::string global_lock_file = PlatformPath::getDocumentsPath( "aribeiro", "lock" ) + PlatformPath::SEPARATOR + this->name + std::string(".llq.f_lock");
                unlink( global_lock_file.c_str() );
            }

            unlock(true);
        #endif
    }

    void PlatformLowLatencyQueueIPC::printStats() {
        printf("[PlatformLowLatencyQueueIPC] Queue Stats\n");

        printf("  semaphore name: %s\n", semaphore_name.c_str());
        printf("  header name: %s\n", header_name.c_str());
        printf("  buffer name: %s\n", buffer_name.c_str());

        printf("  subscribers_count:%u\n", queue_header_ptr->subscribers_count);
        
        printf("  write_pos:%u\n", queue_header_ptr->write_pos);
        printf("  read_pos:%u\n", queue_header_ptr->read_pos);

        printf("  queue_size:%u\n", queue_header_ptr->queue_size);
        printf("  buffer_size:%u\n", queue_header_ptr->buffer_size);

        printf("  capacity:%u\n", queue_header_ptr->capacity);
        printf("  size:%u\n", queue_header_ptr->size);

    }

    bool PlatformLowLatencyQueueIPC::isSignaled()const {
        if (semaphore_ipc == NULL)
            return true;
        return PlatformThread::isCurrentThreadInterrupted();
        //return semaphore_ipc->isSignaled();
    }

}

