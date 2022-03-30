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

#if defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

#include <sys/wait.h>

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

            if (lpApplicationName.find(".") == std::string::npos)
                lpApplicationName += ".exe";

            /*
            TCHAR buffer[MAX_PATH];
            TCHAR** lppPart = { NULL };
            DWORD retval = GetFullPathName(
                lpApplicationName.c_str(),
                MAX_PATH,
                buffer,
                lppPart
            );
            if (retval)
                lpApplicationName = buffer;
            */

            if (commandLine.length() > 0)
                commandLine = "\"" + lpApplicationName + "\"" + " " + commandLine;
            else
                commandLine = "\"" + lpApplicationName + "\"";

            // set the size of the structures
            ZeroMemory(&startupInfo, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            ZeroMemory(&processInformation, sizeof(processInformation));

            // start the program up
            process_created = CreateProcess(NULL, //(LPCTSTR)lpApplicationName.c_str(),   // the path
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

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

            created_pid = fork();
            if (created_pid == 0) {
                // child process
                //execve replaces the current process memory with the new shell executable

                std::vector<std::string> args_str = StringUtil::tokenizer(commandLine, " ");

                char** argv = new char* [args_str.size() + 2];
                memset(argv, 0, (args_str.size() + 2) * sizeof(char*));
                for (int i = 0; i < args_str.size(); i++) {
                    argv[i + 1] = &args_str[i][0];
                }
                argv[0] = &lpApplicationName[0];
                char* const envp[] = { NULL,NULL };

                //printf("Will execute: %s\n",lpApplicationName.c_str());

                execve(lpApplicationName.c_str(), argv, envp);
                perror((std::string("Error to execute: ") + lpApplicationName).c_str());
                exit(127);
            }

            process_created = created_pid > 0;

#endif

        }

        bool waitExit(int* exit_code, uint32_t timeout_ms) {
            PlatformAutoLock autoLock(&mutex);

            if (isCreated()) {

#if defined(_WIN32)
                if (WaitForSingleObject(processInformation.hProcess, timeout_ms) == WAIT_TIMEOUT) {
                    return false;
                }
                DWORD code;
                if (GetExitCodeProcess(processInformation.hProcess, &code)) {
                    *exit_code = (int)code;
                    return true;
                }
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

#endif



            }

            return false;
        }

        bool hasExitCode(int* exit_code) {
            PlatformAutoLock autoLock(&mutex);
            if (isCreated() && !isRunning()) {
#if defined(_WIN32)
                DWORD code;
                if (GetExitCodeProcess(processInformation.hProcess, &code)) {
                    *exit_code = (int)code;
                    return true;
                }
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

#endif

            }
            return false;
        }

        bool isRunning() {
            PlatformAutoLock autoLock(&mutex);
            if (isCreated()) {
#if defined(_WIN32)
                bool isTerminated = WaitForSingleObject(processInformation.hProcess, 0) == WAIT_OBJECT_0;
                return !isTerminated;
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
#endif
            }

            return false;
        }

        bool isCreated() {
            PlatformAutoLock autoLock(&mutex);
            return process_created;
        }

        void horribleForceTermination() {
            PlatformAutoLock autoLock(&mutex);
            if (isCreated()) {
                bool isRunningAux = isRunning();
                exit_code = 0;
                pid_str = "";
                process_created = false;
#if defined(_WIN32)
                if (isRunningAux)
                    TerminateProcess(processInformation.hProcess, exit_code);
                CloseHandle(processInformation.hProcess);
                CloseHandle(processInformation.hThread);
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

                kill(created_pid, SIGKILL);

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

#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

                //SIGTERM or a SIGKILL
                kill(created_pid, sig);

                int status;
                bool timeout = false;

                PlatformTime timer;
                int64_t timer_acc = 0;
                timer.update();
                while (waitpid(created_pid, &status, WNOHANG) != created_pid) {
                    timer.update();
                    timer_acc += timer.deltaTimeMicro;
                    if (timer_acc > force_horrible_terminate_after_ms * 1000) {
                        printf("[PlatformProcess] FORCE KILL CHILDREN TIMEOUT\n");
                        kill(created_pid, SIGKILL);
                        timeout = true;
                        break;
                    }
                    PlatformSleep::sleepMillis(50);
                }

                if (timeout) {
                    exit_code = 0;
                }
                else if (WIFEXITED(status)) {
                    exit_code = WEXITSTATUS(status);
                }
                else
                    exit_code = -1;//exit with error (signal, for example)

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