#include "PlatformProcess.h"

#if defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
    //environment variables location
    extern char ** environ;
#endif

namespace aRibeiro {

        void PlatformProcess::findAndReplaceAll(std::string* data, const std::string& toSearch, const std::string& replaceStr) {
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

    
        PlatformProcess::PlatformProcess(const std::string& _lpApplicationName, const std::string& _commandLine, int _force_horrible_terminate_after_ms) {
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
                int startIndex = 1;
                bool concatenate_next = false;
                for (int i = 0; i < args_str.size(); i++) {
                    if (concatenate_next){
                        concatenate_next = false;

                        args_str[i-1] = args_str[i-1].substr(0,args_str[i-1].length()-1) + " " + args_str[i];

                        argv[startIndex-1] = &args_str[i-1][0];
                    } else 
                        argv[startIndex++] = &args_str[i][0];
                    if (StringUtil::endsWith(args_str[i], "\\")){
                        concatenate_next = true;
                    }
                }

                if (!PlatformPath::isFile(lpApplicationName)){
                    //search in path variable
                    char *dup = strdup(getenv("PATH"));
                    char *s = dup;
                    char *p = NULL;
                    do {
                        p = strchr(s, ':');
                        if (p != NULL) {
                            p[0] = 0;
                        }
                        std::string exe_path = std::string(s)+PlatformPath::SEPARATOR+lpApplicationName;
                        if (PlatformPath::isFile(exe_path)){
                            //check can execute
                            struct stat  st;
                            if (stat(exe_path.c_str(), &st) >= 0){
                                if ((st.st_mode & S_IEXEC) != 0){
                                    printf("Executable found at: %s\n", exe_path.c_str());
                                    lpApplicationName = exe_path;
                                    break;
                                }
                            }

                            //printf("Executable found at: %s\n", exe_path.c_str());
                            //lpApplicationName = exe_path;
                        }
                        s = p + 1;
                    } while (p != NULL);
                    free(dup);
                }

                argv[0] = &lpApplicationName[0];
                //char* const envp[] = { NULL,NULL };
                //printf("Will execute: %s\n",lpApplicationName.c_str());
                execve(lpApplicationName.c_str(), argv, environ);//envp);

                //exit(0);
                perror((std::string("Error to execute: ") + lpApplicationName).c_str());
                
                kill( getpid() , SIGKILL);//SIGABRT);//SIGKILL);
                //exit(127);
            }

            process_created = created_pid > 0;

            printf("process_created %i\n",process_created);

#endif

        }

        bool PlatformProcess::waitExit(int* _exit_code, uint32_t timeout_ms) {
            PlatformAutoLock autoLock(&mutex);

            if (isCreated()) {

#if defined(_WIN32)
                if (WaitForSingleObject(processInformation.hProcess, timeout_ms) == WAIT_TIMEOUT) {
                    return false;
                }
                DWORD code;
                if (GetExitCodeProcess(processInformation.hProcess, &code)) {
                    exit_code = (int)code;
                    *_exit_code = exit_code;
                    return true;
                }
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)


                int status;
                bool timeout = false;

                PlatformTime timer;
                int64_t timer_acc = 0;
                timer.update();
                while (waitpid(created_pid, &status, WNOHANG) != created_pid) {
                    timer.update();
                    timer_acc += timer.deltaTimeMicro;
                    if (timer_acc > timeout_ms * 1000) {
                        timeout = true;
                        break;
                    }
                    PlatformSleep::sleepMillis(50);
                }

                if (timeout) {
                    return false;
                }
                else if (WIFEXITED(status)) {
                    exit_code = WEXITSTATUS(status);
                    *_exit_code = exit_code;

                    //pid_str = "";
                    process_created = false;
                    return true;
                }
                else {
                    exit_code = -1;//exit with error (signal, for example)
                    *_exit_code = exit_code;

                    //pid_str = "";
                    process_created = false;
                    return true;
                }

#endif

            }

            return false;
        }

        int PlatformProcess::getExitCode() {
            return exit_code;
        }

        bool PlatformProcess::isRunning() {
            PlatformAutoLock autoLock(&mutex);
            if (isCreated()) {
#if defined(_WIN32)
                bool isTerminated = WaitForSingleObject(processInformation.hProcess, 0) == WAIT_OBJECT_0;
                if (isTerminated) {
                    DWORD code;
                    if (GetExitCodeProcess(processInformation.hProcess, &code)) {
                        exit_code = (int)code;
                    }

                    CloseHandle(processInformation.hProcess);
                    CloseHandle(processInformation.hThread);

                    //pid_str = "";
                    process_created = false;

                }
                return !isTerminated;
#elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)
                int status;
                // waitpid clear the defunct child processes...
                bool isTerminated = waitpid(created_pid, &status, WNOHANG) == created_pid;
                if (isTerminated){
                    if (WIFEXITED(status))
                        exit_code = WEXITSTATUS(status);
                    else if (WIFSIGNALED(status))
                        exit_code = -1;//WTERMSIG(chld_state)

                    //pid_str = "";
                    process_created = false;
                }
                return !isTerminated;
#endif
            }

            return false;
        }

        bool PlatformProcess::isCreated() {
            PlatformAutoLock autoLock(&mutex);
            return process_created;
        }

        void PlatformProcess::horribleForceTermination() {
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
                //clear defunct
                int status;
                waitpid(created_pid, &status, 0);

#endif
            }
        }

        int PlatformProcess::signal(int sig) {
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
                        //clear defunct
                        waitpid(created_pid, &status, 0);
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

                pid_str = "";
                process_created = false;
#endif
            }

            return exit_code;
        }

        PlatformProcess::~PlatformProcess() {
            signal(SIGINT);
        }

}
