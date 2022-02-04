#ifndef __platform_signal__h__
#define __platform_signal__h__

#include <aRibeiroCore/common.h>

#include <signal.h>

namespace aRibeiro {

class PlatformSignal {

#if defined(_WIN32)
    static void(*fnc)(int);

    static BOOL WINAPI HandlerRoutine_exit( _In_ DWORD dwCtrlType) {
        switch (dwCtrlType)
        {
            // Handle the CTRL-C signal.
        case CTRL_C_EVENT:
            if (PlatformSignal::fnc != NULL)
                PlatformSignal::fnc(0);
            Sleep(5000);
            return TRUE;

            // CTRL-CLOSE: confirm that the user wants to exit.
        case CTRL_CLOSE_EVENT:
            if (PlatformSignal::fnc != NULL)
                PlatformSignal::fnc(0);
            Sleep(5000);
            return TRUE;

            // Pass other signals to the next handler.
        case CTRL_BREAK_EVENT:
            //Beep(900, 200);
            //printf("Ctrl-Break event\n\n");
            return FALSE;

        case CTRL_LOGOFF_EVENT:
            //Beep(1000, 200);
            //printf("Ctrl-Logoff event\n\n");
            return FALSE;

        case CTRL_SHUTDOWN_EVENT:
            //Beep(750, 500);
            //printf("Ctrl-Shutdown event\n\n");
            return FALSE;
        default:
            return FALSE;
        }
    }

#endif

public:
    static void Set(void(*fnc)(int)) {
        signal(SIGINT,  fnc);
        signal(SIGTERM, fnc);

        #if defined(_WIN32)
            PlatformSignal::fnc = fnc;
            SetConsoleCtrlHandler(PlatformSignal::HandlerRoutine_exit, false);
            SetConsoleCtrlHandler(PlatformSignal::HandlerRoutine_exit, true);
        #else
            signal(SIGQUIT, fnc);
        #endif

    }
    static void Reset() {

        signal(SIGINT,  SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        #if defined(_WIN32)
            PlatformSignal::fnc = NULL;
            //SetConsoleCtrlHandler(PlatformSignal::HandlerRoutine_exit, false);
        #else
            signal(SIGQUIT, SIG_DFL);
        #endif
    }
};

}

#endif