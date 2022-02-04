#include "PlatformSignal.h"

namespace aRibeiro {

#if defined(_WIN32)
    void(*PlatformSignal::fnc)(int) = NULL;
#endif

    void PlatformSignal::Set(void(*fnc)(int)) {
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
    void PlatformSignal::Reset() {

        signal(SIGINT,  SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        #if defined(_WIN32)
            PlatformSignal::fnc = NULL;
            //SetConsoleCtrlHandler(PlatformSignal::HandlerRoutine_exit, false);
        #else
            signal(SIGQUIT, SIG_DFL);
        #endif
    }

}

