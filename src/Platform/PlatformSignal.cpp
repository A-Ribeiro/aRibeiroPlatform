#include "PlatformSignal.h"

namespace aRibeiro {

#if defined(_WIN32)
    void(*PlatformSignal::fnc)(int) = NULL;
#endif

}

