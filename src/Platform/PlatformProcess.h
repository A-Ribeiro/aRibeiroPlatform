#ifndef Platform_Process_h__
#define Platform_Process_h__

#include <aRibeiroCore/common.h>
#include <aRibeiroCore/StringUtil.h>
#include <aRibeiroPlatform/PlatformLowLatencyQueueIPC.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformPath.h>
#include <string>
#include <algorithm>
#include <signal.h>

#if defined(OS_TARGET_win)

#include <aRibeiroPlatform/WindowsPipe.h>

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

#include <sys/wait.h>
#include <aRibeiroPlatform/UnixPipe.h>

#endif


namespace aRibeiro {

#if defined(_WIN32)
    //
    // This Class is needed only in windows systems.... 
    // that does not have a way to signaling a process...
    //
    class PlatformProcessGracefulWindowsProcessTerminator {

        DWORD dwProcessId;
        std::string pid_str;

        PlatformLowLatencyQueueIPC* queue;

        PlatformThread thread;

        void(*fnc)(int);

        void threadRun() {

            ObjectBuffer signal;
            queue->read(&signal);
            if (queue->isSignaled())
                return;

            int signal_int = SIGINT;

            if (signal.size >= 4)
                signal_int = *(int*)signal.data;

            fnc(signal_int);
            //raise(signal_int);
        }

    public:
        PlatformProcessGracefulWindowsProcessTerminator(void(*_fnc)(int)) :
            thread(this, &PlatformProcessGracefulWindowsProcessTerminator::threadRun),
            fnc(_fnc) {
            queue = NULL;
            dwProcessId = GetCurrentProcessId();

            char aux[64];
            sprintf(aux, "p%u", dwProcessId);
            pid_str = aux;

            queue = new PlatformLowLatencyQueueIPC(pid_str.c_str(), PlatformQueueIPC_READ, 16, sizeof(int));
            thread.setShouldDisposeThreadByItself(true);
            thread.start();
        }

        ~PlatformProcessGracefulWindowsProcessTerminator() {
            thread.interrupt();
            thread.wait();
        }

    };
#endif

    class PlatformProcess {

        PlatformMutex mutex;
        std::string lpApplicationName;
        std::string commandLine;

        bool process_created;

        std::string pid_str;

        int exit_code;

        int force_horrible_terminate_after_ms;

#if defined(_WIN32)
        STARTUPINFO startupInfo;
        PROCESS_INFORMATION processInformation;
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        pid_t created_pid;

#endif

    public:

        static bool ApplicationExists(const std::string& _lpApplicationName);

        PlatformProcess(const std::string& _lpApplicationName, const std::vector<std::string>& vector_argv, int _force_horrible_terminate_after_ms = 5000
#if defined(OS_TARGET_win)
            , WindowsPipe* pipe_stdin = NULL, WindowsPipe* pipe_stdout = NULL, WindowsPipe* pipe_stderr = NULL
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
            ,UnixPipe *pipe_stdin = NULL, UnixPipe *pipe_stdout = NULL, UnixPipe *pipe_stderr = NULL
#endif
        );
        bool waitExit(int* exit_code, uint32_t timeout_ms);
        int getExitCode();
        bool isRunning();
        bool isCreated();
        void horribleForceTermination();
        int signal(int sig = SIGINT);
        ~PlatformProcess();
    };

}

#endif