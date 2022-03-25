#ifndef __Global_Thread_Options_H
#define __Global_Thread_Options_H

#include <aRibeiroCore/common.h>

namespace aRibeiro {

    enum GlobalThreadPriority{
        GlobalThreadPriority_None,
        GlobalThreadPriority_Normal,
        GlobalThreadPriority_High,
        GlobalThreadPriority_Realtime
    };

    // this will affect all threads created after this call
    void setGlobalThreadPriority(GlobalThreadPriority priority);
    GlobalThreadPriority getGlobalThreadPriority();
    
}



#endif
