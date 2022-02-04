#ifndef _platform_autolock_h__
#define _platform_autolock_h__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformMutex.h>

namespace aRibeiro {

    class PlatformAutoLock {
        PlatformMutex *mutex;
    public:
        PlatformAutoLock(PlatformMutex *mutex){
            this->mutex = mutex;
            this->mutex->lock();
        }
        ~PlatformAutoLock() {
            this->mutex->unlock();
        }
    };

}


#endif