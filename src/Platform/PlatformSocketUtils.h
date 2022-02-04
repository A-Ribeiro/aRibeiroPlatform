#ifndef __platform_socket__utils__h__
#define __platform_socket__utils__h__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <aRibeiroPlatform/PlatformSemaphore.h>
#include <aRibeiroPlatform/PlatformSleep.h>
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroPlatform/NetworkConstants.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <string>

#ifndef _WIN32

#include <errno.h>

#include <sys/socket.h>
#include <sys/times.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#undef closesocket
#define closesocket(s)	close(s)

#else

//#include <comdef.h>
#ifndef MSG_DONTWAIT
    #define MSG_DONTWAIT 0
#endif

#endif

#include <sys/types.h>

namespace aRibeiro {

    // Class used in windows to initialize and finalize sockets
    class SocketUtils {
#ifdef _WIN32
        bool startup_ok;
        PlatformMutex mutex;
#endif
        SocketUtils();
    public:
        void InitSockets();
        ~SocketUtils();
        static SocketUtils *Instance();

        static std::string getLastSocketErrorMessage();
        static std::string getLastErrorMessage();

        // Returns true on success, or false if there was an error
        static bool SetSocketBlockingEnabled(int fd, bool blocking);

        static uint32_t ipv4_address_to_nl(const std::string &ip_address) {
            uint32_t result;

            if (ip_address.size() == 0 || ip_address.compare("INADDR_ANY") == 0)
                result = htonl(INADDR_ANY);
            else if (ip_address.compare("INADDR_LOOPBACK") == 0)
                result = htonl(INADDR_LOOPBACK);
            else
                result = inet_addr(ip_address.c_str());

            return result;
        }

        static struct sockaddr_in mountAddress(const std::string &ip = "127.0.0.1", uint16_t port = NetworkConstants::PUBLIC_PORT_START) {
            struct sockaddr_in result = { 0 };

            result.sin_family = AF_INET;
            result.sin_addr.s_addr = SocketUtils::ipv4_address_to_nl(ip);
            result.sin_port = htons(port);

            return result;
        }
    };

}

#endif