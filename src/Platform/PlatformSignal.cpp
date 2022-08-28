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

    OnAbortDelegate ___default_delegate;

    void __second_level_abort_fnc(const char* file, int line, const char* format, ...) {
        fprintf(stderr, "ERROR: Called abort inside the abort event...\n");
        va_list args;
        va_start(args, format);
        aRibeiro::DefaultAbortFNC(file,line,format,args);
        va_end(args);
    }

    void PlatformSignal::DefaultAbortFNC(const char* file, int line, const char* format, ...) {
        fprintf(stderr, "PlatformSignal::DefaultAbortFNC\n");

        va_list args;

        std::vector<char> char_buffer;
        va_start(args, format);
        char_buffer.resize(vsnprintf(NULL, 0, format, args) + 1);
        va_end(args);

        va_start(args, format);
        int len = vsnprintf(&char_buffer[0], char_buffer.size(), format, args);
        va_end(args);

        aRibeiro::OnAbortFNC = __second_level_abort_fnc;
        ___default_delegate(file, line, &char_buffer[0]);

        aRibeiro::DefaultAbortFNC(file,line,"%s",&char_buffer[0]);
        exit(-1);
    }

    OnAbortDelegate* PlatformSignal::OnAbortEvent() {
        aRibeiro::OnAbortFNC = PlatformSignal::DefaultAbortFNC;
        return &___default_delegate;
    }

}

