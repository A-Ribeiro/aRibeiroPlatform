#include "PlatformThread.h"
#include "PlatformSleep.h"
#include "PlatformTime.h"

#include <stdio.h>

#if defined(OS_TARGET_win)
    #include <process.h>

#elif defined(OS_TARGET_linux)
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <signal.h>
    #include <pthread.h>

    #define GetCurrentThreadId() syscall(SYS_gettid)

    void platform_thread_signal_handler_usr1(int signal) {
    }

#elif defined(OS_TARGET_mac)
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <signal.h>
    #include <pthread.h>

    uint64_t GetCurrentThreadId() {
        uint64_t tid;
        pthread_threadid_np(NULL, &tid);
        return tid;
    }

    void platform_thread_signal_handler_usr1(int signal) { }

#else
    #error Thread for this platform is not implemented...
#endif

namespace aRibeiro {
    
    
    struct ThreadFinalizeStruct
    {
        THREAD_HANDLE_TYPE handle;
#if defined(OS_TARGET_win)
        HANDLE interrupt_event_handle;
#endif
        ThreadRunEntryPoint *runEntryPoint;
        int32_t timeout_ms;
    };

    volatile bool ReleaseThreadIDClass_finished = false;

    class ReleaseThreadIDClass{

        PlatformMutex threadsToFinalizeLock;
        std::vector<ThreadFinalizeStruct> threadsToFinalize;
        PlatformThread releaseThread;
    
        void release(const ThreadFinalizeStruct &aux){
            
            //printf("ReleaseThreadIDClass::release() release thread ID\n");
            
#if defined(OS_TARGET_win)
            if (aux.handle != NULL) {
                WaitForSingleObject(aux.handle, INFINITE);
                if (aux.handle)
                    CloseHandle(aux.handle);
                //aux.handle = NULL;
            }
            //CLOSE EVENT
            if (aux.interrupt_event_handle != NULL)
                CloseHandle(aux.interrupt_event_handle);
            //aux.interrupt_event_handle = NULL;
#else
            pthread_join(aux.handle, NULL);
#endif
            if (aux.runEntryPoint != NULL)
                delete aux.runEntryPoint;
            //aux.runEntryPoint = NULL;
        }
        
        void releaseThreadID_thread(){
            PlatformTime time;
            
            time.update();
            while ( !PlatformThread::isCurrentThreadInterrupted() ) {
                time.update();
                int32_t delta_ms = time.deltaTimeMicro/1000;
                threadsToFinalizeLock.lock();
                for(int i=threadsToFinalize.size()-1;i>=0;i--){
                    threadsToFinalize[i].timeout_ms -= delta_ms;
                    if (threadsToFinalize[i].timeout_ms < 0){
                        ThreadFinalizeStruct aux = threadsToFinalize[i];
                        threadsToFinalize.erase(threadsToFinalize.begin()+i);
                        release(aux);
                    }
                }
                threadsToFinalizeLock.unlock();
                PlatformSleep::sleepMillis(300);
            }
            
            threadsToFinalizeLock.lock();
            for(int i=threadsToFinalize.size()-1;i>=0;i--){
                release(threadsToFinalize[i]);
            }
            threadsToFinalize.clear();
            threadsToFinalizeLock.unlock();
            
        }
    public:
        
        ReleaseThreadIDClass():releaseThread(this, &ReleaseThreadIDClass::releaseThreadID_thread){
            releaseThread.setShouldDisposeThreadByItself(true);
            releaseThread.start();
        }
        
        virtual ~ReleaseThreadIDClass(){
            ReleaseThreadIDClass_finished = true;
            printf("~ReleaseThreadIDClass()\n");
            releaseThread.interrupt();
            releaseThread.wait();
        }
        
        void registerToReleaseThreadID( ThreadFinalizeStruct v ){
            if (ReleaseThreadIDClass_finished) {
                printf("ERROR: trying to release a thread after the ReleaseThreadIDClass has been released...\n");
                return;
            }
            threadsToFinalizeLock.lock();
            threadsToFinalize.push_back(v);
            threadsToFinalizeLock.unlock();
        }
        
        static ReleaseThreadIDClass* Instance(){
            static ReleaseThreadIDClass instance;
            return &instance;
        }
        
    };
    
    
    
    //
    // GLOBALS STATIC
    //
    //PlatformMutex PlatformThread::threadsToFinalizeLock;
    //std::vector<ThreadFinalizeStruct> PlatformThread::threadsToFinalize;

    class PlatformThread_OpenedThreadManager {
    public:
        PlatformMutex openedThreadsLock;
        std::map<THREAD_ID_TYPE, PlatformThread*> openedThreads;
        void registerThread(PlatformThread* thread) {
            THREAD_ID_TYPE tid = GetCurrentThreadId();
            openedThreadsLock.lock();
            openedThreads[tid] = thread;
            openedThreadsLock.unlock();
        }

        void unregisterThread(PlatformThread* thread) {
            THREAD_ID_TYPE tid = GetCurrentThreadId();
            std::map<THREAD_ID_TYPE, PlatformThread*>::iterator it;
            openedThreadsLock.lock();
            if (openedThreads.size() > 0) {
                it = openedThreads.find(tid);
                if (it != openedThreads.end()) {
                    PlatformThread* result = it->second;
                    openedThreads.erase(it);
                    result->exited = true;
                }
            }
            openedThreadsLock.unlock();
        }

        PlatformThread* getThreadByID(const THREAD_ID_TYPE &tid) {
            PlatformThread* result = NULL;
            openedThreadsLock.lock();
            if (openedThreads.size() > 0) {
                std::map<THREAD_ID_TYPE, PlatformThread*>::iterator it;
                it = openedThreads.find(tid);
                if (it != openedThreads.end())
                    result = it->second;
            }
            openedThreadsLock.unlock();
            return result;
        }


        static PlatformThread_OpenedThreadManager *Instance() {
            static PlatformThread_OpenedThreadManager manager;
            return &manager;
        }

    };
    
    bool PlatformThread::isCurrentThreadInterrupted() {
        PlatformThread* thread = getCurrentThread();
        if (thread != NULL)
            return thread->interrupted;
        return false;
    }

    PlatformThread* PlatformThread::getCurrentThread() {
        PlatformThread* result = PlatformThread_OpenedThreadManager::Instance()->getThreadByID(GetCurrentThreadId());
        if (result == NULL)
            return PlatformThread::getMainThread();
        return result;
    }

    PlatformThread* PlatformThread::getMainThread() {
        //printf("[PlatformThread] Get main thread");
            //return main thread
        // force to instanciate the opened thread manager before the main thread
        //   Fix finalization issues related to the main thread...
        PlatformThread_OpenedThreadManager::Instance();
        static PlatformThread mainThread;
        if (mainThread.name.size() == 0)
            mainThread.name = "Main Thread";
        return &mainThread;
    }



    #if ! defined(OS_TARGET_win)

    void PlatformThread::self_interrupt_thread_kill_semaphores( void* platform_thread ) {

        //openedThreadsLock.lock();

        PlatformThread *_instance = (PlatformThread *)platform_thread;


        /*
        bool valid = false;
        std::map<THREAD_ID_TYPE, PlatformThread*>::iterator it = openedThreads.begin();
        while (it != openedThreads.begin()) {
            if (it->second == _instance) {
                valid = true;
                break;
            }
            it++;
        }
        */

        //if (!valid) {
        //    openedThreadsLock.unlock();
        //    return ;
        //}


        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = platform_thread_signal_handler_usr1;
        sa.sa_flags = 0;
        sigaction(SIGUSR1, &sa, 0);

        _instance->mutex.lock();

        printf("Trying to signal a thread with USR1. pthread_t %p\n", _instance->m_thread);
        pthread_kill(*_instance->getNativeThread(), SIGUSR1);
        while (_instance->opened_semaphore > 0) {

            _instance->mutex.unlock();
            
            aRibeiro::PlatformSleep::sleepMillis(50);

            _instance->mutex.lock();
            //printf("Trying to signal a thread with USR1. pthread_t %p\n", _instance->m_thread);
            pthread_kill(*_instance->getNativeThread(), SIGUSR1);
        }

        _instance->mutex.unlock();

        //openedThreadsLock.unlock();
    }

    #endif

    //
    // GLOBALS - NON STATIC
    //
    void PlatformThread::interrupt() {

        mutex.lock();

#if !defined(OS_TARGET_win)
        if (skip_interrupt){
            mutex.unlock();
            return;
        }
#endif

        if (interrupted){
            mutex.unlock();
            return;
        }

        interrupted = true;


#if defined(OS_TARGET_win)
        
        SetEvent(m_thread_interrupt_event);

#else

    if (m_isMain || m_isActive) {



            /*
            for (int i=0;i<opened_semaphores.size();i++){
                //force return to execution all locked semaphores...
                sem_post(opened_semaphores[i]);
            }
            */

            /*
            std::map<sem_t*, bool>::iterator it;
            for (it = opened_semaphores.begin(); it != opened_semaphores.end(); it++)
                sem_post(it->first);
            opened_semaphores.clear();
            */

            //std::vector<sem_t*> opened_semaphores;

            
            //
            // Generate EINTR on wait state of semaphores and similar wait objects
            //
            

            if ( //m_isMain && 
            pthread_self() == m_thread ) {

                //pthread_kill(m_thread, SIGUSR1);

                //raise(SIGUSR1);

                /*
                // weird situation on linux, the signal is sent in a loop...
                while (opened_semaphore > 0) {
                    mutex.unlock();

                    aRibeiro::PlatformSleep::sleepMillis(500);

                    printf("[Raise] Trying to signal a thread with USR1...\n");

                    mutex.lock();
                    raise(SIGUSR1);
                }
                */

               printf("Creating interrupt thread. pthread_t %p\n", m_thread);

                if (interrupt_thread == NULL) {
                    interrupt_thread = new PlatformThread(self_interrupt_thread_kill_semaphores, this);
                    interrupt_thread->name = "Interrupt Thread";
                    interrupt_thread->setShouldDisposeThreadByItself(true);
                    interrupt_thread->skip_interrupt = true;
                    interrupt_thread->start();
                }

            } else {
                //raise(SIGUSR1);//raise on local thread

                // check is main thread
                //if (m_thread == )

                struct sigaction sa;
                sigemptyset(&sa.sa_mask);
                sa.sa_handler = platform_thread_signal_handler_usr1;
                sa.sa_flags = 0;
                sigaction(SIGUSR1, &sa, 0);

                printf("[MainThread] Trying to signal a thread with USR1...\n");
                pthread_kill(m_thread, SIGUSR1);
                while (opened_semaphore > 0) {
                    mutex.unlock();

                    aRibeiro::PlatformSleep::sleepMillis(50);

                    //printf("[MainThread] Trying to signal a thread with USR1...\n");

                    mutex.lock();
                    pthread_kill(m_thread, SIGUSR1);
                }
                // */

                /*
                if (interrupt_thread == NULL) {
                    interrupt_thread = new PlatformThread(self_interrupt_thread_kill_semaphores, this);
                    interrupt_thread->setShouldDisposeThreadByItself(true);
                    interrupt_thread->skip_interrupt = true;
                    interrupt_thread->start();
                }
                */

            }

           


            
    }

#endif

        mutex.unlock();
    }

    PlatformThread::~PlatformThread() {

#if !defined(OS_TARGET_win)
        mutex.lock();
        if (interrupt_thread != NULL){
            mutex.unlock();
            interrupt_thread->wait();
            mutex.lock();
            if (interrupt_thread != NULL)
                delete interrupt_thread;
            interrupt_thread = NULL;
        } else
            mutex.unlock();
#endif

#if !defined(OS_TARGET_win)
        // avoid call interrupt in the static main thread instance...
        m_isMain = false;
#endif

        interrupt();
        wait();
        
        if (shouldReleaseThreadID_byItself){
            printf("PlatformThread::~PlatformThread() shouldReleaseThreadID_byItself = true\n");
            PlatformSleep::sleepMillis(10);
            while (waitCalls>0)
                PlatformSleep::sleepMillis(10);
            
#if defined(OS_TARGET_win)
            if (m_thread)
            {
                // A thread cannot wait for itself!
                if (m_threadId != GetCurrentThreadId())
                    WaitForSingleObject(m_thread, INFINITE);
                
                //CLOSE THREAD
                if (m_thread)
                    CloseHandle(m_thread);
                m_thread = NULL;
            }

            //CLOSE EVENT
            if (m_thread_interrupt_event != NULL)
                CloseHandle(m_thread_interrupt_event);
            m_thread_interrupt_event = NULL;
        
#else
        
            if (m_isActive)
            {
                // A thread cannot wait for itself!
                if (pthread_equal(pthread_self(), m_thread) == 0)
                    pthread_join(m_thread, NULL);
                
                //m_thread = NULL;
            }
        
#endif
        
            if (runEntryPoint != NULL)
                delete runEntryPoint;
            runEntryPoint = NULL;
        } else {
            
            ThreadFinalizeStruct aux;
#if defined(OS_TARGET_win)
            aux.interrupt_event_handle = m_thread_interrupt_event;
#endif
            aux.handle = m_thread;
            aux.runEntryPoint = runEntryPoint;
            aux.timeout_ms = 1000;
            
            
#if defined(OS_TARGET_win)
            if (m_thread)
#else
            if (m_isActive)
#endif
                ReleaseThreadIDClass::Instance()->registerToReleaseThreadID(aux);
            else {
                if (runEntryPoint != NULL)
                    delete runEntryPoint;
                runEntryPoint = NULL;
            }
            
        }
        
    }

    void PlatformThread::start() {
        
        mutex.lock();
        if (started) {
            printf("thread already started...\n");
            mutex.unlock();
            return;
        }
        started = true;
        mutex.unlock();

        //force register the main thread...
        PlatformThread::getMainThread();
        
        // create OS thread
#if defined(OS_TARGET_win)
        m_thread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &PlatformThread::entryPoint, this, 0, &m_threadId));

        if (!m_thread)
        {
            printf("Erro ao abrir thread...\n");
        }

#else
        bool error;

        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        
        error = pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE) != 0;
        ARIBEIRO_ABORT(error, "Error set Thread Detach Scope\n");

        error = pthread_attr_setscope(&attrs, PTHREAD_SCOPE_SYSTEM) != 0;
        ARIBEIRO_ABORT(error, "Error set Thread Scope\n");

        //error = pthread_attr_setschedpolicy(&attrs, SCHED_FIFO);
        error = pthread_attr_setschedpolicy(&attrs, SCHED_RR) != 0;
        ARIBEIRO_ABORT(error, "Error set Thread Policy\n");

        // when set RoundRobin or FIFO, the priority can be 1 .. 99
        struct sched_param param = { 0 };
        param.sched_priority = 1;
        error = pthread_attr_setschedparam(&attrs, &param) != 0;
        ARIBEIRO_ABORT(error, "Error set Thread Priority\n");
        
        m_isActive = pthread_create(&m_thread, &attrs, &PlatformThread::entryPoint, this) == 0;
        if (!m_isActive)
        {
            printf("Erro ao abrir thread...\n");
        }
#endif

    }
    
    /*
    void PlatformThread::disposeThread(PlatformThread* t) {
        ThreadFinalizeStruct structFinalize;
        structFinalize.handle = t->m_thread;
        structFinalize.runEntryPoint = t->runEntryPoint;
        structFinalize.timeout = 1000;
        threadsToFinalizeLock.lock();
        threadsToFinalize.push_back(structFinalize);
        threadsToFinalizeLock.unlock();
    }*/
    
    

    void PlatformThread::wait() {

        //self thread test
#if defined(OS_TARGET_win)
        if (!m_thread)
            return;
        if (m_threadId == GetCurrentThreadId())
            return;
#else
        if (!m_isActive)
            return;
        if (pthread_equal(pthread_self(), m_thread) != 0)
            return;
#endif

        waitCalls++;
        while (isAlive())
            PlatformSleep::sleepMillis(1);
        waitCalls--;
    }
    
    bool PlatformThread::isAlive() {
#if defined(OS_TARGET_win)
        return !exited && m_thread != NULL;
#else
        return !exited && m_isActive;
#endif
    }
    
    bool PlatformThread::isStarted(){
        return started;
    }
    
    void PlatformThread::setShouldDisposeThreadByItself(bool v) {
        shouldReleaseThreadID_byItself = v;
    }

    void PlatformThread::terminate() {
        /*
        if (m_impl)
        {
            m_impl->terminate();
            delete m_impl;
            m_impl = NULL;
        }*/

#if defined(OS_TARGET_win)
        if (m_thread)
            TerminateThread(m_thread, 0);
        //CLOSE THREAD
        if (m_thread)
            CloseHandle(m_thread);
        m_thread = NULL;
#else
        if (m_isActive)
        {
#ifndef OS_ANDROID
            pthread_cancel(m_thread);
#else
            // See http://stackoverflow.com/questions/4610086/pthread-cancel-al
            pthread_kill(m_thread, SIGUSR1);
#endif
        }
#endif
    }

    //void PlatformThread::run()
    //{
    //    runEntryPoint->run();
    //}

#if defined(OS_TARGET_win)

    unsigned int __stdcall PlatformThread::entryPoint(void* userData)
    {
        // The Thread instance is stored in the user data
        PlatformThread* owner = static_cast<PlatformThread*>(userData);
        PlatformThread_OpenedThreadManager::Instance()->registerThread(owner);
        // Forward to the owner
        owner->runEntryPoint->run();
        PlatformThread_OpenedThreadManager::Instance()->unregisterThread(owner);
        // Optional, but it is cleaner
        _endthreadex(0);
        return 0;
    }

#else
    void* PlatformThread::entryPoint(void* userData)
    {
        // The Thread instance is stored in the user data
        PlatformThread* owner = static_cast<PlatformThread*>(userData);

#ifndef OS_ANDROID
        // Tell the thread to handle cancel requests immediately
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif
        PlatformThread_OpenedThreadManager::Instance()->registerThread(owner);
        // Forward to the owner
        owner->runEntryPoint->run();
        PlatformThread_OpenedThreadManager::Instance()->unregisterThread(owner);
        return NULL;
    }
#endif
    

}
