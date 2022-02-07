#include "PlatformSleep.h"
#include "PlatformTime.h"

#if defined(OS_TARGET_win)
#include <timeapi.h>
#endif

namespace aRibeiro {
#if defined(OS_TARGET_win)
    class UseWindowsHighResolutionClock {
    public:
        UseWindowsHighResolutionClock() {
            timeBeginPeriod(1);
        }
        ~UseWindowsHighResolutionClock() {
            timeEndPeriod(1);
        }
        static UseWindowsHighResolutionClock* Instance() {
            static UseWindowsHighResolutionClock useWindowsHighResolutionClock;
            return &useWindowsHighResolutionClock;
        }
    };
#endif

    void PlatformSleep::sleepSec(int secs) {
        sleepMillis(secs * 1000);
    }

    void PlatformSleep::sleepMillis(int millis) {
#if defined(OS_TARGET_win)
        UseWindowsHighResolutionClock::Instance();
        Sleep(millis);
#else
        usleep(millis * 1000);
#endif
    }

    void PlatformSleep::busySleepMicro(int64_t micros) {

        /*
        int64_t micro = 0;
        PlatformTime time;
        do {
            time.update();
            micro += time.deltaTimeMicro;
        } while (micro < micros);
        */


#if defined(OS_TARGET_win)

        w32PerformanceCounter counter;
        while (counter.GetCounterMicro(false) < micros)
        {

        }

#else

        usleep(micros);

#endif

    }

}
