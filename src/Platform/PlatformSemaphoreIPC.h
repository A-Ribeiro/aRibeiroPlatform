#ifndef _platform_semaphore_ipc_h__
#define _platform_semaphore_ipc_h__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformSleep.h>


#if defined(OS_TARGET_win)
    #define WIN32_LEAN_AND_MEAN

    #include <WinSock2.h>
    #include <WS2tcpip.h>

    #include <windows.h>

    static std::string _GetLastErrorToString_semaphore_ipc() {
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

    #include <stdio.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <unistd.h>
    #include <signal.h>

#else
    #error Semaphore for this platform is not implemented...
#endif


namespace aRibeiro {

    // On linux the Semaphore IPC are persistent... 
    // you need to create a logic with file_lock (lockf)
    // to ensure the correct initialization
    class PlatformSemaphoreIPC {
        
        //bool signaled;

    #if defined(OS_TARGET_win)
        HANDLE semaphore;
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
        sem_t* semaphore;
    #endif

        //private copy constructores, to avoid copy...
        PlatformSemaphoreIPC(const PlatformSemaphoreIPC& v){}
        void operator=(const PlatformSemaphoreIPC& v){}

    public:
    
        std::string name;

        PlatformSemaphoreIPC(const std::string &name, int count, bool truncate = false) {
            this->name = name;
            //signaled = false;
    #if defined(OS_TARGET_win)
            SECURITY_DESCRIPTOR sd;
            InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
            SetSecurityDescriptorDacl(&sd,TRUE,(PACL)0,FALSE);

            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = &sd;
            sa.bInheritHandle = FALSE;

            semaphore = CreateSemaphoreA(
                &sa,         // default security attributes
                count,       // initial count
                LONG_MAX,    //count,  // maximum count
                this->name.c_str() // unnamed semaphore
            );
            ARIBEIRO_ABORT(semaphore == NULL, "CreateSemaphore error: %s\n", _GetLastErrorToString_semaphore_ipc().c_str());
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            //sem_init(&semaphore, 0, count);// 0 means is a semaphore bound to threads

            this->name = std::string("/") + this->name;

            if (truncate) {
                sem_unlink(this->name.c_str());
            }

            semaphore = sem_open(
                this->name.c_str(),
                O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                count
            );

            /*
            int v;
            sem_getvalue(semaphore, &v);
            
            if(truncate) {
                while (v > count) {
                    sem_wait(semaphore);
                    sem_getvalue(semaphore, &v);
                }
                while (v < count) {
                    sem_post(semaphore);
                    sem_getvalue(semaphore, &v);
                }
            }*/

            //printf("SEMAPHORE VALUE: %i\n",v);

            ARIBEIRO_ABORT(semaphore == SEM_FAILED, "Error to create global semaphore. Error code: %s\n", strerror(errno));
    #endif
        }
        virtual ~PlatformSemaphoreIPC(){
    #if defined(OS_TARGET_win)
            if (semaphore != NULL)
                CloseHandle(semaphore);
            semaphore = NULL;
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            //sem_destroy(&semaphore);
            if (semaphore != NULL)
                sem_close(semaphore);
            //sem_unlink(this->name.c_str());
            semaphore = NULL;
    #endif
        }

        bool tryToAcquire(uint32_t timeout_ms = 0) {
            //printf("[Semaphore] tryToAquire...\n");

            aRibeiro::PlatformThread *currentThread = aRibeiro::PlatformThread::getCurrentThread();

#if defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            currentThread->semaphoreLock();
#endif

            //if (signaled || currentThread->isCurrentThreadInterrupted()) {
            if (isSignaled()) {
                //signaled = true;
#if defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
                currentThread->semaphoreUnLock();
#endif
                return false;
            }

    #if defined(OS_TARGET_win)


            DWORD dwWaitResult;

            //dwWaitResult = WaitForSingleObject( semaphore, (DWORD)timeout_ms );
            //signaled = signaled || dwWaitResult == WAIT_FAILED;

            HANDLE handles_threadInterrupt_sem[2] = {
                semaphore, // WAIT_OBJECT_0 + 0
                currentThread->m_thread_interrupt_event // WAIT_OBJECT_0 + 1
            };

            dwWaitResult = WaitForMultipleObjects(
                2,   // number of handles in array
                handles_threadInterrupt_sem,     // array of thread handles
                FALSE,          // wait until all are signaled
                (DWORD)timeout_ms //INFINITE
            );

            // true if the interrupt is signaled (and only the interrupt...)
            if (dwWaitResult == WAIT_OBJECT_0 + 1) {
                //signaled = true;
                return false;
            }

            // true if the semaphore is signaled (might have the interrupt or not...)
            return dwWaitResult == WAIT_OBJECT_0 + 0;
    #elif defined(OS_TARGET_linux)
            if (timeout_ms == 0) {
                currentThread->semaphoreUnLock();
                return sem_trywait(semaphore) == 0;
            }

            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts)) {
                currentThread->semaphoreUnLock();
                ARIBEIRO_ABORT(true, "clock_gettime error\n");
            }

            struct timespec ts_increment;
            ts_increment.tv_sec = timeout_ms / 1000;
            ts_increment.tv_nsec = ((long)timeout_ms % 1000L) * 1000000L;

            ts.tv_nsec += ts_increment.tv_nsec;
            ts.tv_sec += ts.tv_nsec / 1000000000L;
            ts.tv_nsec = ts.tv_nsec % 1000000000L;

            ts.tv_sec += ts_increment.tv_sec;

            currentThread->semaphoreWaitBegin(semaphore);
            currentThread->semaphoreUnLock();

//printf("wait for 10 s\n");
//PlatformSleep::sleepMillis(10000);
//printf("10 s done...\n");


            int s = sem_timedwait(semaphore, &ts);
            
            currentThread->semaphoreWaitDone(semaphore);

            // currentThread->isCurrentThreadInterrupted() || 
            if ( (s == -1 && errno != ETIMEDOUT) ) {
                //printf(". s: %s \n", strerror(errno));

                //interrupt signaled
                //signaled = true;
                return false;
            }

            return s == 0;
    #elif defined(OS_TARGET_mac)
            if (timeout_ms == 0) {
                currentThread->semaphoreUnLock();
                return sem_trywait(semaphore) == 0;
            }

            currentThread->semaphoreWaitBegin(semaphore);
            currentThread->semaphoreUnLock();

            int s = -1;
            int timeout_int = (int)timeout_ms;
            while(!currentThread->isCurrentThreadInterrupted() && timeout_int > 0) {
                s = sem_trywait(semaphore);
                if (s == 0)
                    break;
                timeout_int -= 1;
                PlatformSleep::sleepMillis(1);
            }

            currentThread->semaphoreWaitDone(semaphore);

            return s == 0;
    #endif
        }

        bool blockingAcquire() {
#if defined(OS_TARGET_win)
            bool signaled = isSignaled();
            while (!signaled && !tryToAcquire(UINT32_MAX)) {
                signaled = isSignaled();
            }
            return !signaled;
            /*
            if (signaled || aRibeiro::PlatformThread::getCurrentThread()->isCurrentThreadInterrupted())
                signaled = true;
            else
                while (!signaled && !tryToAcquire(UINT32_MAX));
            return !signaled;
            */
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            aRibeiro::PlatformThread *currentThread = aRibeiro::PlatformThread::getCurrentThread();

            
            /*
            if (signaled || aRibeiro::PlatformThread::getCurrentThread()->isCurrentThreadInterrupted())
                signaled = true;
            else {
                while (!signaled && !tryToAcquire( 60000 )) { //UINT32_MAX
                    //PlatformSleep::sleepMillis(1);
                    //printf(". signaled: %i \n", signaled);
                }
            }
            // */

            /*

            currentThread->semaphoreLock();
            if (signaled || currentThread->isCurrentThreadInterrupted()) {
                signaled = true;
                currentThread->semaphoreUnLock();
            } else {
                currentThread->semaphoreWaitBegin(semaphore);
                currentThread->semaphoreUnLock();
                signaled = signaled || (sem_wait(semaphore) != 0);
                currentThread->semaphoreWaitDone(semaphore);
                //signaled = signaled || currentThread->isCurrentThreadInterrupted();
            }

            // */

            currentThread->semaphoreLock();
            
            bool signaled = isSignaled();
            
            if (signaled) {
                //signaled = true;
                currentThread->semaphoreUnLock();
            }
            else {
                currentThread->semaphoreWaitBegin(semaphore);
                currentThread->semaphoreUnLock();
                signaled = signaled || (sem_wait(semaphore) != 0);
                currentThread->semaphoreWaitDone(semaphore);
                //signaled = signaled || currentThread->isCurrentThreadInterrupted();
            }

            return !signaled;
#endif
        }

        void release() {
    #if defined(OS_TARGET_win)
            //printf("[Semaphore] release...\n");
            BOOL result = ReleaseSemaphore( semaphore, 1, NULL );
            ARIBEIRO_ABORT(!result, "ReleaseSemaphore error: %s\n", _GetLastErrorToString_semaphore_ipc().c_str());
    #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            sem_post(semaphore);
    #endif
        }

        // only check if this queue is signaled for the current thread... 
        // it may be active in another thread...
        bool isSignaled() const {
            return PlatformThread::isCurrentThreadInterrupted();
            //return signaled;
        }

        void forceUnlinkLinux() {

    #if !defined(OS_TARGET_win)
            if (semaphore != NULL)
                sem_unlink( name.c_str() );
    #endif

        }

    };

}


#endif