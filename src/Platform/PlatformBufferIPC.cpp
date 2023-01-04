#include "PlatformBufferIPC.h"
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformSleep.h>
#include <aRibeiroPlatform/PlatformPath.h>
#include <aRibeiroPlatform/PlatformSignal.h>

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


    void PlatformBufferIPC::lock(bool from_constructor) {
        PlatformAutoLock autoLock(&shm_mutex);

#if defined(OS_TARGET_win)
        // lock semaphore
        ARIBEIRO_ABORT(WaitForSingleObject(buffer_semaphore, INFINITE) != WAIT_OBJECT_0, "Error to lock queue semaphore. Error code: %s\n", GetLastErrorToString().c_str());
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        if (from_constructor) {

            ARIBEIRO_ABORT(f_lock != -1, "Trying to lock twice from constructor.\n");

            // file lock ... to solve the dead semaphore reinitialization...
            std::string global_lock_file = PlatformPath::getDocumentsPath( "aribeiro", "lock" ) + PlatformPath::SEPARATOR + this->name + std::string(".b.f_lock");

            f_lock = open(global_lock_file.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            ARIBEIRO_ABORT(f_lock == -1, "Error to open f_lock. Error code: %s\n", strerror(errno));

            int rc = lockf(f_lock, F_LOCK, 0);
            ARIBEIRO_ABORT(rc == -1, "Error to lock f_lock. Error code: %s\n", strerror(errno));

            
        } else {
            // while semaphore is signaled, try to aquire until block...
            if (buffer_semaphore != NULL)
                while (sem_wait(buffer_semaphore) != 0);
        }
#endif
    }

    void PlatformBufferIPC::unlock(bool from_constructor) {
#if defined(OS_TARGET_win)
        // release semaphore
        ARIBEIRO_ABORT(!ReleaseSemaphore(buffer_semaphore, 1, NULL), "Error to unlock queue semaphore. Error code: %s\n", GetLastErrorToString().c_str());
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        if (from_constructor) {
            ARIBEIRO_ABORT(f_lock == -1, "Trying to unlock a non initialized lock from constructor.\n");

            int rc = lockf(f_lock, F_ULOCK, 0);
            ARIBEIRO_ABORT(rc == -1, "Error to unlock f_lock. Error code: %s\n", strerror(errno));

            close(f_lock);
            f_lock = -1;
        }
        else {
            if (buffer_semaphore != NULL)
                sem_post(buffer_semaphore);
        }
#endif
    }

    PlatformBufferIPC::PlatformBufferIPC(
        const char* name,
        //uint32_t mode,
        uint32_t buffer_size_
    ) {

        //PlatformAutoLock autoLock(&shm_mutex);
        shm_mutex.lock();
        PlatformSignal::OnAbortEvent()->add(this, &PlatformBufferIPC::onAbort);

        force_finish_initialization = true;

        size = buffer_size_;

        buffer_semaphore = NULL;
        buffer_handle = BUFFER_HANDLE_NULL;

        this->name = name;

#if defined(OS_TARGET_win)
        buffer_name = std::string(name) + std::string("_abd");//aribeiro_buffer_data
        semaphore_name = std::string(name) + std::string("_abs");//aribeiro_buffer_semaphore
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        buffer_name = std::string("/") + std::string(name) + std::string("_abd");//aribeiro_buffer_data
        semaphore_name = std::string("/") + std::string(name) + std::string("_abs");//aribeiro_buffer_semaphore

        f_lock = -1;
#endif

        //printf("semaphore_name: %s\n", semaphore_name.c_str());
        //printf("buffer_name: %s\n", buffer_name.c_str());
        
#if defined(OS_TARGET_win)
        SECURITY_DESCRIPTOR sd;
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd,TRUE,(PACL)0,FALSE);
        
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = FALSE;

        buffer_semaphore = CreateSemaphoreA(
            &sa, // default security attributes
            1, // initial count
            LONG_MAX, // maximum count
            semaphore_name.c_str() // named semaphore
        );
        ARIBEIRO_ABORT(buffer_semaphore == 0, "Error to create global semaphore. Error code: %s\n", GetLastErrorToString().c_str() );
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
/*
        buffer_semaphore = sem_open(
            semaphore_name.c_str(), 
            O_CREAT, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            1
        );

        ARIBEIRO_ABORT(buffer_semaphore == SEM_FAILED, "Error to create global semaphore. Error code: %s\n", strerror(errno) );
*/
#endif

        lock(true);

#if defined(OS_TARGET_win)
        // open the buffer memory section
        buffer_handle = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size + sizeof(uint32_t), buffer_name.c_str() );
        if (buffer_handle == 0) {
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the buffer IPC queue.\n");
        }
        
        //if (mode == ( PlatformBufferIPC_READ | PlatformBufferIPC_WRITE) ) {
            real_data_ptr = (uint8_t*)MapViewOfFile(buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, size + sizeof(uint32_t));
            if (real_data_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        /*} else if (mode == PlatformBufferIPC_READ) {
            real_data_ptr = (uint8_t*)MapViewOfFile(buffer_handle, FILE_MAP_READ, 0, 0, size + sizeof(uint32_t));
            if (real_data_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        } else if (mode == PlatformBufferIPC_WRITE) {
            real_data_ptr = (uint8_t*)MapViewOfFile(buffer_handle, FILE_MAP_WRITE, 0, 0, size + sizeof(uint32_t));
            if (real_data_ptr == 0) {
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer.\n");
            }
        } else {
            unlock(true);
            ARIBEIRO_ABORT(true, "Buffer opening mode not specified.\n");
        }*/

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        buffer_handle = shm_open(buffer_name.c_str(), 
            O_CREAT | O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (buffer_handle == -1) {
            buffer_handle = BUFFER_HANDLE_NULL;
            unlock(true);
            ARIBEIRO_ABORT(true, "Error to create the buffer IPC queue. Error code: %s\n", strerror(errno) );
        }

        int rc;

        struct stat _stat;
        rc = fstat(buffer_handle, &_stat);
        ARIBEIRO_ABORT(rc != 0, "Error to stat the file descriptor. Error code: %s\n", strerror(errno) );
        if (_stat.st_size == 0) {
            rc = ftruncate(buffer_handle, size + sizeof(uint32_t));
            //fallocate(buffer_handle, 0, 0, size + sizeof(uint32_t));
            ARIBEIRO_ABORT(rc != 0, "Error to truncate buffer. Error code: %s\n", strerror(errno) );

            // truncate semaphore and lock it...
            sem_unlink(semaphore_name.c_str());
        }

        // just initialize the semaphore
        buffer_semaphore = sem_open(
            semaphore_name.c_str(),
            O_CREAT, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            1
        );

        if (buffer_semaphore == SEM_FAILED) {
            buffer_semaphore = NULL;
            ARIBEIRO_ABORT(true, "Error to create global semaphore. Error code: %s\n", strerror(errno) );
        }

        lock();//lock the created semaphore

        //if (mode == (PlatformBufferIPC_READ | PlatformBufferIPC_WRITE)) {
            real_data_ptr = (uint8_t*)mmap(
                NULL,
                size + sizeof(uint32_t),
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                buffer_handle,
                0
            );
            if (real_data_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno) );
            }
        /*}
        else if (mode == PlatformBufferIPC_READ) {
            real_data_ptr = (uint8_t*)mmap(
                NULL,
                size + sizeof(uint32_t),
                PROT_READ,
                MAP_SHARED,
                buffer_handle,
                0
            );
            if (real_data_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno) );
            }
        }
        else if (mode == PlatformBufferIPC_WRITE) {
            real_data_ptr = (uint8_t*)mmap(
                NULL,
                size + sizeof(uint32_t),
                PROT_WRITE,
                MAP_SHARED,
                buffer_handle,
                0
            );
            if (real_data_ptr == MAP_FAILED) {
                unlock();
                unlock(true);
                ARIBEIRO_ABORT(true, "Error to map the IPC buffer. Error code: %s\n", strerror(errno) );
            }
        }
        else {
            unlock();
            unlock(true);
            ARIBEIRO_ABORT(true, "Buffer opening mode not specified.\n");
        }*/

#endif

        uint32_t *count = (uint32_t *)&real_data_ptr[size];
        (*count) ++;

        isFirst = (*count) == 1;

        data = &real_data_ptr[ 0 ];

        //printf("Initialization OK.\n");
        
        //unlock();
    }

    bool PlatformBufferIPC::isFirstProcess(){
        return isFirst;
    }

    void PlatformBufferIPC::finishInitialization() {
        //ARIBEIRO_ABORT(!isFirst, "Calling finishInitialization on a non-first memory initialization process.\n");
        #if !defined(OS_TARGET_win)
            unlock();
        #endif

        unlock(true);

        force_finish_initialization = false;
        shm_mutex.unlock();
    }

    PlatformBufferIPC::~PlatformBufferIPC() {
        releaseAll();
    }
    
    void PlatformBufferIPC::onAbort(const char *file, int line, const char *message){
        PlatformAutoLock autoLock(&shm_mutex);
        if (force_finish_initialization)
            finishInitialization();
        releaseAll();
    }
    
    void PlatformBufferIPC::releaseAll(){
        PlatformAutoLock autoLock(&shm_mutex);

        PlatformSignal::OnAbortEvent()->remove(this, &PlatformBufferIPC::onAbort);

        #if !defined(OS_TARGET_win)
            lock(true);
            bool is_last_buffer = false;

            if (buffer_handle != BUFFER_HANDLE_NULL) {
                uint32_t *count = (uint32_t *)&real_data_ptr[size];
                is_last_buffer = ((*count)-1) == 0;
            }
        #endif

        lock();

        if (buffer_handle != BUFFER_HANDLE_NULL) {

            uint32_t *count = (uint32_t *)&real_data_ptr[size];
            (*count)--;

#if defined(OS_TARGET_win)
            if (real_data_ptr != 0)
                UnmapViewOfFile(real_data_ptr);
            CloseHandle(buffer_handle);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            if (real_data_ptr != MAP_FAILED)
                munmap(real_data_ptr, size + sizeof(uint32_t));
            close(buffer_handle); //close FD
            //shm_unlink(buffer_name.c_str());
#endif
            buffer_handle = BUFFER_HANDLE_NULL;
            real_data_ptr = NULL;
            data = NULL;
        }

        unlock();

        if (buffer_semaphore != NULL) {
#if defined(OS_TARGET_win)
            CloseHandle(buffer_semaphore);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            sem_close(buffer_semaphore);
            //sem_unlink(semaphore_name.c_str());
#endif
            buffer_semaphore = NULL;

        #if !defined(OS_TARGET_win)
            // unlink all resources
            if (is_last_buffer) {
                printf("[linux] ... last buffer, unlink resources ...\n");
                shm_unlink(buffer_name.c_str());
                sem_unlink(semaphore_name.c_str());

                //unlink the lock_f
                std::string global_lock_file = PlatformPath::getDocumentsPath( "aribeiro", "lock" ) + PlatformPath::SEPARATOR + this->name + std::string(".b.f_lock");
                unlink( global_lock_file.c_str() );
            }

            unlock(true);
        #endif
        }
    }

}

