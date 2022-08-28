#ifndef __platform_signal__h__
#define __platform_signal__h__

#include <aRibeiroCore/common.h>

#include <signal.h>

namespace aRibeiro {

    BEGIN_DECLARE_DELEGATE(OnAbortDelegate, const char* file, int line, const char* message) CALL_PATTERN(file, line, message) END_DECLARE_DELEGATE;

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
        
        case CTRL_BREAK_EVENT:
            //Beep(900, 200);
            //printf("Ctrl-Break event\n\n");
            if (PlatformSignal::fnc != NULL)
                PlatformSignal::fnc(0);
            Sleep(5000);
            return TRUE;

            // Pass other signals to the next handler.
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

    static void Set(void(*fnc)(int));
    static void Reset();

    static void DefaultAbortFNC(const char* file, int line, const char* format, ...);
    static OnAbortDelegate* OnAbortEvent();
};

}

#endif