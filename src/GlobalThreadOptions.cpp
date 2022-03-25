#include "GlobalThreadOptions.h"
#include <aRibeiroPlatform/PlatformThread.h>

namespace aRibeiro {


    GlobalThreadPriority __thread_priority = GlobalThreadPriority_None;

    void setGlobalThreadPriority(GlobalThreadPriority priority) {

        /*ARIBEIRO_ABORT(
            PlatformThread::getCurrentThread() != PlatformThread::getMainThread(),
            "You can set the thread priority just from the main thread.\n"
        );*/

        __thread_priority = priority;

        #if defined(OS_TARGET_win)

            if (__thread_priority == GlobalThreadPriority_Normal){
                if (!SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS))
                    printf("SetPriorityClass  MainThread ERROR...\n");
                if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
                    printf("SetThreadPriority MainThread ERROR...\n");
            }
            else if (__thread_priority == GlobalThreadPriority_High){
                if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
                    printf("SetPriorityClass  MainThread ERROR...\n");
                if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
                    printf("SetThreadPriority MainThread ERROR...\n");
            }
            else if (__thread_priority == GlobalThreadPriority_Realtime){
                if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
                    printf("SetPriorityClass  MainThread ERROR...\n");
                if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
                    printf("SetThreadPriority MainThread ERROR...\n");
            }

        #elif defined(OS_TARGET_linux) || defined(OS_TARGET_mac)

        #endif

        if (__thread_priority == GlobalThreadPriority_Normal){
            printf("setGlobalThreadPriority -> GlobalThreadPriority_Normal\n");
        }
        else if (__thread_priority == GlobalThreadPriority_High){
            printf("setGlobalThreadPriority -> GlobalThreadPriority_High\n");
        }
        else if (__thread_priority == GlobalThreadPriority_Realtime){
            printf("setGlobalThreadPriority -> GlobalThreadPriority_Realtime\n");
        }

    }

    GlobalThreadPriority getGlobalThreadPriority() {
        if (__thread_priority == GlobalThreadPriority_None)
            setGlobalThreadPriority(GlobalThreadPriority_Normal);
        return __thread_priority;
    }
}