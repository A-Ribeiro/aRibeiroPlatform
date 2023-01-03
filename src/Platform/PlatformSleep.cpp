#include "PlatformSleep.h"
#include "PlatformTime.h"

#if defined(OS_TARGET_win)
    #include <timeapi.h>
    #include <processthreadsapi.h>
#else
    #include <sched.h>
    #include <errno.h>
#endif

namespace aRibeiro {
#if defined(OS_TARGET_win)
    class UseWindowsHighResolutionClock {
        UseWindowsHighResolutionClock() {
            timeBeginPeriod(1);
        }
    public:
        ~UseWindowsHighResolutionClock() {
            timeEndPeriod(1);
        }
        static void sleep(int millis) {
            static UseWindowsHighResolutionClock useWindowsHighResolutionClock;
            Sleep(millis);
        }
    };
#endif

    void PlatformSleep::sleepSec(int secs) {
        sleepMillis(secs * 1000);
    }

    void PlatformSleep::sleepMillis(int millis) {
#if defined(OS_TARGET_win)
        UseWindowsHighResolutionClock::sleep(millis);
        //Sleep(millis);
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


    void PlatformSleep::yield(){

#if defined(OS_TARGET_win)
        //yield();
        //UseWindowsHighResolutionClock::sleep(0);
        if (!SwitchToThread())
            UseWindowsHighResolutionClock::sleep(0);
#else
        //usleep(0) << same effect...
        int rc = sched_yield();
        ARIBEIRO_ABORT(rc!=0, "Error: %s", strerror( errno ));
#endif
        
    }

}
