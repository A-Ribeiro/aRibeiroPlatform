#ifndef Platform_Process_h__
#define Platform_Process_h__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformLowLatencyQueueIPC.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformPath.h>
#include <string>
#include <algorithm>
#include <signal.h>

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

            queue = new PlatformLowLatencyQueueIPC(pid_str.c_str(), PlatformQueueIPC_WRITE, 16, sizeof(int));
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
#endif

        static void findAndReplaceAll(std::string* data, const std::string& toSearch, const std::string& replaceStr) {
            // Get the first occurrence
            size_t pos = data->find(toSearch);
            // Repeat till end is reached
            while (pos != std::string::npos) {
                // Replace this occurrence of Sub String
                data->replace(pos, toSearch.size(), replaceStr);
                // Get the next occurrence from the current position
                pos = data->find(toSearch, pos + replaceStr.size());
            }
        }

    public:
        PlatformProcess(const std::string& _lpApplicationName, const std::string& _commandLine, int _force_horrible_terminate_after_ms = 5000) {
            exit_code = 0;
            pid_str = "";
            process_created = false;
            lpApplicationName = _lpApplicationName;
            commandLine = _commandLine;
            force_horrible_terminate_after_ms = _force_horrible_terminate_after_ms;

#if defined(_WIN32)

            findAndReplaceAll(&lpApplicationName, "/", PlatformPath::SEPARATOR);
            lpApplicationName += ".exe";

            if (commandLine.length() > 0)
                commandLine = lpApplicationName + " " + commandLine;
            else
                commandLine = lpApplicationName;

            // set the size of the structures
            ZeroMemory(&startupInfo, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            ZeroMemory(&processInformation, sizeof(processInformation));

            // start the program up
            process_created = CreateProcess((LPCTSTR)lpApplicationName.c_str(),   // the path
                (LPSTR)&commandLine[0],        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0, //CREATE_NEW_PROCESS_GROUP, // No creation flags // CREATE_NEW_CONSOLE |
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory 
                &startupInfo,            // Pointer to STARTUPINFO structure
                &processInformation             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
            );

            if (isCreated()) {
                char aux[64];
                sprintf(aux, "p%u", processInformation.dwProcessId);
                pid_str = aux;
            }

#endif

        }

        bool isCreated() {
            PlatformAutoLock autoLock(&mutex);
            return process_created;
        }

        void horribleForceTermination() {
            PlatformAutoLock autoLock(&mutex);
            if (isCreated()) {
                exit_code = 0;
                pid_str = "";
                process_created = false;
#if defined(_WIN32)
                TerminateProcess(processInformation.hProcess, exit_code);
                CloseHandle(processInformation.hProcess);
                CloseHandle(processInformation.hThread);
#endif
            }
        }

        int signal(int sig = SIGINT) {
            PlatformAutoLock autoLock(&mutex);

            if (isCreated()) {
#if defined(_WIN32)

                //check the process signal queue
                PlatformLowLatencyQueueIPC queue(pid_str.c_str(), PlatformQueueIPC_WRITE, 16, sizeof(int));
                queue.write((uint8_t*)&sig, sizeof(int), false);

                if (WaitForSingleObject(processInformation.hProcess, force_horrible_terminate_after_ms) == WAIT_TIMEOUT) {
                    // if the process did not finish in the time
                    //   Forcibly terminate the process...
                    TerminateProcess(processInformation.hProcess, 0);
                }

                DWORD code;
                GetExitCodeProcess(processInformation.hProcess, &code);

                CloseHandle(processInformation.hProcess);
                CloseHandle(processInformation.hThread);

                //ARIBEIRO_ABORT(code != 0, "Child process terminated with code: %u", code);
                exit_code = code;

                pid_str = "";
                process_created = false;

#endif
            }

            return exit_code;
        }

        ~PlatformProcess() {
            signal(SIGINT);
        }
    };

}

#endif