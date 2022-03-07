#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <map>
#include <string>

// Fix for unaligned stack with clang and GCC on Windows XP 32-bit
#if defined(OS_TARGET_win) && (defined(__clang__) || defined(__GNUC__))
    #define ALIGN_STACK __attribute__((__force_align_arg_pointer__))
#else
    #define ALIGN_STACK
#endif

#if defined(OS_TARGET_win)
    #define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
    #include <windows.h>
    typedef DWORD THREAD_ID_TYPE;

    typedef HANDLE THREAD_HANDLE_TYPE;


    static std::string _GetLastErrorToString_thread() {
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

#elif defined(OS_TARGET_linux) || defined(ARIBEIRO_OS_ANDROID)
    #include <algorithm>
    #include <semaphore.h>

    #include <sys/syscall.h>
    #include <sys/types.h>
    typedef pid_t THREAD_ID_TYPE;

    typedef pthread_t THREAD_HANDLE_TYPE;
#elif defined(OS_TARGET_mac)
    #include <algorithm>
    #include <semaphore.h>
    #include <sys/types.h>
    typedef uint64_t THREAD_ID_TYPE;

    typedef pthread_t THREAD_HANDLE_TYPE;
#else
    #error Thread for this platform is not implemented...
#endif

namespace aRibeiro {

    //
    // Thread Function templates...
    //

    /// \brief Base class to hold the thread entry point reference
    ///
    /// \author Alessandro Ribeiro
    ///
    struct ThreadRunEntryPoint
    {
        virtual ~ThreadRunEntryPoint() {}
        virtual void run() = 0;
    };

    /// \brief Functor entry point specialization
    ///
    /// \author Alessandro Ribeiro
    ///
    template <typename T>
    struct ThreadRunFunctor : ThreadRunEntryPoint
    {
        ThreadRunFunctor(T functor) : m_functor(functor) {}
        virtual void run() { m_functor(); }
        T m_functor;
    };

    /// \brief Functor entry point specialization with one argument
    ///
    /// \author Alessandro Ribeiro
    ///
    template <typename F, typename A>
    struct ThreadRunFunctorWithArg : ThreadRunEntryPoint
    {
        ThreadRunFunctorWithArg(F function, A arg) : m_function(function), m_arg(arg) {}
        virtual void run() { m_function(m_arg); }
        F m_function;
        A m_arg;
    };

    /// \brief Method entry point specialization
    ///
    /// \author Alessandro Ribeiro
    ///
    template <typename C>
    struct ThreadRunMemberFunc : ThreadRunEntryPoint
    {
        ThreadRunMemberFunc(void(C::*function)(), C* object) : m_function(function), m_object(object) {}
        virtual void run() { (m_object->*m_function)(); }
        void(C::*m_function)();
        C* m_object;
    };

    /// \brief Method entry point specialization with an argument
    ///
    /// \author Alessandro Ribeiro
    ///
    template <typename C, typename A>
    struct ThreadRunMemberFuncWithArg : ThreadRunEntryPoint
    {
        ThreadRunMemberFuncWithArg(void(C::*function)(), C* object, A arg) : m_function(function), m_object(object), m_argument(arg){}
        virtual void run() { (m_object->*m_function)(m_argument); }
        void(C::*m_function)();
        C* m_object;
        A m_argument;
    };

    class PlatformThread_OpenedThreadManager;
    
    /// \brief Manager a thread in several platforms. You can use functions, structs and classes.
    ///
    /// \author Alessandro Ribeiro
    ///
    class PlatformThread
    {
    private:

        //
        // OS SPECIFIC ATTRIBUTES
        //
#if defined(OS_TARGET_win)

        ALIGN_STACK static unsigned int __stdcall entryPoint(void* userData);
        HANDLE m_thread;
        unsigned int m_threadId;

    public:

        HANDLE m_thread_interrupt_event;
#else
        static void* entryPoint(void* userData);
        pthread_t m_thread;
        bool      m_isActive;
        bool      m_isMain;

        PlatformThread *interrupt_thread;

        bool skip_interrupt;

    public:

        pthread_t *getNativeThread() {
            return &m_thread;
        }

        //std::map<sem_t*, bool> opened_semaphores;

        int opened_semaphore;

        void semaphoreLock() {
            mutex.lock();
        }

        void semaphoreUnLock() {
            mutex.unlock();
        }

        void semaphoreWaitBegin(sem_t *semaphore) {
            mutex.lock();
            opened_semaphore++;
            //opened_semaphores[semaphore] = true;
            mutex.unlock();
        }

        void semaphoreWaitDone(sem_t *semaphore) {
            mutex.lock();
            opened_semaphore--;
            /*
            //opened_semaphores.erase(std::remove(opened_semaphores.begin(), opened_semaphores.end(), semaphore), opened_semaphores.end());
            std::map<sem_t*, bool>::iterator it = opened_semaphores.find(semaphore);
            if (it != opened_semaphores.end())
                opened_semaphores.erase(it);
                */
            mutex.unlock();
        }
        
#endif

    private:
        
        PlatformMutex mutex;
        
        volatile bool interrupted;
        volatile bool exited;
        volatile bool started;
        volatile bool shouldReleaseThreadID_byItself;
        
        volatile int waitCalls;

        ThreadRunEntryPoint *runEntryPoint;

        #if ! defined(OS_TARGET_win)
            static void self_interrupt_thread_kill_semaphores( void* platform_thread );
        #endif


    public:
        
        std::string name;

        /// \brief Check if the current thread is interrupted
        ///
        /// A thread can be interrupted by calling interrupt() method from the parent thread.
        ///
        /// Example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_example() {
        ///     // the thread or main thread could call #interrupt() to change this state
        ///     while (!PlatformThread::isCurrentThreadInterrupted()) {
        ///         ...
        ///         //avoid 100% CPU using by this thread
        ///         PlatformSleep::sleepMillis(1);
        ///     }
        /// }
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        /// \return true if the thread is interrupted
        ///
        static bool isCurrentThreadInterrupted();

        /// \brief Get the current Thread
        ///
        /// Get the current thread. If the thread is not spawned by the PlatformThread, it will return null.
        ///
        /// \author Alessandro Ribeiro
        /// \return A reference to the current thread spawned.
        ///
        static PlatformThread* getCurrentThread();
        static PlatformThread* getMainThread();

        virtual ~PlatformThread();

        /// \brief Set a flag to the thread indicating that it is interrupted.
        ///
        /// Example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_example() {
        ///     // the thread or main thread could call #PlatformThread::interrupt() to change this state
        ///     while (!PlatformThread::isCurrentThreadInterrupted()) {
        ///         ...
        ///         //avoid 100% CPU using by this thread
        ///         PlatformSleep::sleepMillis(1);
        ///     }
        /// }
        ///
        /// PlatformThread thread( &thread_example );
        /// // start the thread
        /// thread.start();
        /// ...
        /// // inside the thread implementation, it is necessary to check the #PlatformThread::isCurrentThreadInterrupted()
        /// thread.interrupt();
        /// // wait thread execution finish
        /// thread.wait();
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        void interrupt();

        /// \brief Spawn the thread. It can be called just once.
        ///
        /// \brief Set a flag to the thread indicating that it is interrupted.
        ///
        /// Example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_example() {
        ///     // the thread or main thread could call #PlatformThread::interrupt() to change this state
        ///     while (!PlatformThread::isCurrentThreadInterrupted()) {
        ///         ...
        ///         //avoid 100% CPU using by this thread
        ///         PlatformSleep::sleepMillis(1);
        ///     }
        /// }
        ///
        /// PlatformThread thread( &thread_example );
        /// // start the thread
        /// thread.start();
        /// ...
        /// // inside the thread implementation, it is necessary to check the #PlatformThread::isCurrentThreadInterrupted()
        /// thread.interrupt();
        /// // wait thread execution finish
        /// thread.wait();
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        void start();

        /// \brief Wait the current thread to terminate their execution scope. Can be called once.
        ///
        /// Example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_example() {
        ///     // the thread or main thread could call #PlatformThread::interrupt() to change this state
        ///     while (!PlatformThread::isCurrentThreadInterrupted()) {
        ///         ...
        ///         //avoid 100% CPU using by this thread
        ///         PlatformSleep::sleepMillis(1);
        ///     }
        /// }
        ///
        /// PlatformThread thread( &thread_example );
        /// // start the thread
        /// thread.start();
        /// ...
        /// // inside the thread implementation, it is necessary to check the #PlatformThread::isCurrentThreadInterrupted()
        /// thread.interrupt();
        /// // wait thread execution finish
        /// thread.wait();
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        void wait();
        bool isAlive();
        bool isStarted();
        void setShouldDisposeThreadByItself(bool v);

        /// \brief Force the target thread to terminate. It does not garantee the execution scope to terminate.
        ///
        /// \warning It is dangerous to call this method.
        ///
        /// Example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_example() {
        ///     ...
        /// }
        ///
        /// PlatformThread thread( &thread_example );
        /// // start the thread
        /// thread.start();
        /// ...
        /// thread.terminate();
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        void terminate();

        /// \brief Constructor (function)
        ///
        /// Creates a thread associated with a function as its run point.
        ///
        /// example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void thread_function()
        /// {
        ///     ...
        /// }
        ///
        /// PlatformThread thread( &thread_function );
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        template <typename F>
        PlatformThread(F function) {
            runEntryPoint = new ThreadRunFunctor<F>(function);
            interrupted = false;
            exited = false;
            started = false;
            shouldReleaseThreadID_byItself = false;
            waitCalls = 0;
#if defined(OS_TARGET_win)
            m_thread = NULL;
            m_thread_interrupt_event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL//TEXT("WriteEvent")  // object name
            );
            ARIBEIRO_ABORT(m_thread_interrupt_event == NULL, "CreateEvent error: %s\n", _GetLastErrorToString_thread().c_str());
#else
            m_isActive = false;
            m_isMain = false;
            opened_semaphore = 0;
            interrupt_thread = NULL;
            skip_interrupt = false;
#endif
            getMainThread();
        }

        /// \brief Constructor (function, argument)
        ///
        /// Creates a thread associated with a function as its run point.
        /// Can pass an argument to the thread.
        ///
        /// example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// void threadFunc(int argument) {
        ///     ...
        /// }
        ///
        /// PlatformThread thread(&threadFunc, 5);
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        template <typename F, typename A>
        PlatformThread(F function, A argument) {
            runEntryPoint = new ThreadRunFunctorWithArg<F, A>(function, argument);
            interrupted = false;
            exited = false;
            started = false;
            shouldReleaseThreadID_byItself = false;
            waitCalls = 0;
#if defined(OS_TARGET_win)
            m_thread = NULL;
            m_thread_interrupt_event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL//TEXT("WriteEvent")  // object name
            );
            ARIBEIRO_ABORT(m_thread_interrupt_event == NULL, "CreateEvent error: %s\n", _GetLastErrorToString_thread().c_str());
#else
            m_isActive = false;
            m_isMain = false;
            opened_semaphore = 0;
            interrupt_thread = NULL;
            skip_interrupt = false;
#endif
            getMainThread();
        }

        /// \brief Constructor (method, object)
        ///
        /// Creates a thread associated with a method as its run point.
        /// Need to pass the object as the second parameter
        ///
        /// example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// class Task {
        /// public:
        ///     void run()
        ///     {
        ///         ...
        ///     }
        /// };
        ///
        /// Task task_instance;
        ///
        /// PlatformThread thread(&task_instance, &Task::run);
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        template <typename C>
        PlatformThread(C* object, void(C::*function)()) {
            runEntryPoint = new ThreadRunMemberFunc<C>(function, object);
            interrupted = false;
            exited = false;
            started = false;
            shouldReleaseThreadID_byItself = false;
            waitCalls = 0;
#if defined(OS_TARGET_win)
            m_thread = NULL;
            m_thread_interrupt_event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL//TEXT("WriteEvent")  // object name
            );
            ARIBEIRO_ABORT(m_thread_interrupt_event == NULL, "CreateEvent error: %s\n", _GetLastErrorToString_thread().c_str());
#else
            m_isActive = false;
            m_isMain = false;
            opened_semaphore = 0;
            interrupt_thread = NULL;
            skip_interrupt = false;
#endif
            getMainThread();
        }

        /// \brief Constructor (method, object, argument)
        ///
        /// Creates a thread associated with a method as its run point.
        /// Need to pass the object as the second parameter
        ///
        /// example:
        ///
        /// \code
        /// #include <aRibeiroPlatform/aRibeiroPlatform.h>
        /// using namespace aRibeiro;
        ///
        /// class Task {
        /// public:
        ///     void run(int arg)
        ///     {
        ///         ...
        ///     }
        /// };
        ///
        /// Task task_instance;
        ///
        /// PlatformThread thread(&task_instance, &Task::run, 5);
        /// \endcode
        ///
        /// \author Alessandro Ribeiro
        ///
        template <typename C, typename A>
        PlatformThread(C* object, void(C::*function)(), A argument) {
            runEntryPoint = new ThreadRunMemberFuncWithArg<C,A>(function, object, argument);
            interrupted = false;
            exited = false;
            started = false;
            shouldReleaseThreadID_byItself = false;
            waitCalls = 0;
#if defined(OS_TARGET_win)
            m_thread = NULL;
            m_thread_interrupt_event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL//TEXT("WriteEvent")  // object name
            );
            ARIBEIRO_ABORT(m_thread_interrupt_event == NULL, "CreateEvent error: %s\n", _GetLastErrorToString_thread().c_str());
#else
            m_isActive = false;
            m_isMain = false;
            opened_semaphore = 0;
            interrupt_thread = NULL;
            skip_interrupt = false;
#endif
            getMainThread();//force get the main thread ID and thread instance...
        }

private:
        //constructor for main thread
        PlatformThread(){
            runEntryPoint = NULL;
            interrupted = false;
            exited = false;
            started = false;
            shouldReleaseThreadID_byItself = false;
            waitCalls = 0;
#if defined(OS_TARGET_win)
            m_threadId = GetCurrentThreadId();
            m_thread = NULL;
            m_thread_interrupt_event = CreateEvent(
                NULL,               // default security attributes
                TRUE,               // manual-reset event
                FALSE,              // initial state is nonsignaled
                NULL//TEXT("WriteEvent")  // object name
            );
            ARIBEIRO_ABORT(m_thread_interrupt_event == NULL, "CreateEvent error: %s\n", _GetLastErrorToString_thread().c_str());
#else
            m_thread = pthread_self();
            m_isActive = false;
            m_isMain = true;
            opened_semaphore = 0;
            interrupt_thread = NULL;
            skip_interrupt = false;

            setShouldDisposeThreadByItself(true);
#endif
        }

public:
        friend class PlatformThread_OpenedThreadManager;

        static int QueryNumberOfSystemThreads() {
        // https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
        #if defined(OS_TARGET_win)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            int numCPU = sysinfo.dwNumberOfProcessors;
            return numCPU;
        #elif defined(OS_TARGET_linux)
            int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
            return numCPU;
        #elif defined(OS_TARGET_mac)
            int mib[4];
            int numCPU;
            std::size_t len = sizeof(numCPU); 

            /* set the mib for hw.ncpu */
            mib[0] = CTL_HW;
            mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

            /* get the number of CPUs from the system */
            sysctl(mib, 2, &numCPU, &len, NULL, 0);

            if (numCPU < 1) 
            {
                mib[1] = HW_NCPU;
                sysctl(mib, 2, &numCPU, &len, NULL, 0);
                if (numCPU < 1)
                    numCPU = 1;
            }
            return numCPU;
        #else
            #error "QueryNumberOfSystemCores not implemented in the current system"
        #endif
        }

    };

}



#endif
