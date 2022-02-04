#include "PlatformSocketUtils.h"
#include <aRibeiroPlatform/PlatformAutoLock.h>
#include <aRibeiroCore/StringUtil.h>


namespace aRibeiro {

    // private constructor...
    SocketUtils::SocketUtils() {
    }

    void SocketUtils::InitSockets() {
#ifdef _WIN32
        PlatformAutoLock auto_lock(&mutex);
        if (startup_ok)
            return;
        WORD version;
        WSADATA wsaData;
        version = MAKEWORD(2, 2);
        startup_ok = (WSAStartup(version, &wsaData) == 0);
#endif
    }

    SocketUtils::~SocketUtils() {
#ifdef _WIN32
        PlatformAutoLock auto_lock(&mutex);
        if (startup_ok) {
            WSACleanup();
            startup_ok = false;
        }
#endif
    }

    SocketUtils *SocketUtils::Instance() {
        static SocketUtils result;
        return &result;
    }



    std::string SocketUtils::getLastSocketErrorMessage() {
        std::string result;

#if defined(_WIN32)
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);

        // WCHAR TO CONSOLE CP (Code Page)
        if (s) {
            /*
            wchar_t *aux = new wchar_t[lstrlenW(s)+1];
            memset(aux, 0, (lstrlenW(s)+1) * sizeof(wchar_t));

            //MultiByteToWideChar(CP_ACP, 0L, s, lstrlenA(s) + 1, aux, strlen(s));
            MultiByteToWideChar(
                CP_OEMCP //GetConsoleCP()
                , MB_PRECOMPOSED | MB_ERR_INVALID_CHARS , s, lstrlenA(s) + 1, aux, lstrlenA(s));
                result = StringUtil::toString(aux);
            */
            
            char *aux = new char[lstrlenW(s) + 1];
            memset(aux, 0, (lstrlenW(s) + 1) * sizeof(char));
            WideCharToMultiByte(
                GetConsoleOutputCP(), //GetConsoleCP(),
                WC_COMPOSITECHECK,
                s, lstrlenW(s) + 1, 
                aux, lstrlenW(s),
                NULL,NULL
            );
            result = aux;
            delete[] aux;
            LocalFree(s);
        }
#else
        result = strerror(errno);
#endif

        return result;
    }

    std::string SocketUtils::getLastErrorMessage() {
        std::string result;

#if defined(_WIN32)
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);

        // WCHAR TO CONSOLE CP (Code Page)
        if (s) {
            /*
            wchar_t *aux = new wchar_t[lstrlenW(s)+1];
            memset(aux, 0, (lstrlenW(s)+1) * sizeof(wchar_t));

            //MultiByteToWideChar(CP_ACP, 0L, s, lstrlenA(s) + 1, aux, strlen(s));
            MultiByteToWideChar(
                CP_OEMCP //GetConsoleCP()
                , MB_PRECOMPOSED | MB_ERR_INVALID_CHARS , s, lstrlenA(s) + 1, aux, lstrlenA(s));
                result = StringUtil::toString(aux);
            */

            char *aux = new char[lstrlenW(s) + 1];
            memset(aux, 0, (lstrlenW(s) + 1) * sizeof(char));
            WideCharToMultiByte(
                GetConsoleOutputCP(), //GetConsoleCP(),
                WC_COMPOSITECHECK,
                s, lstrlenW(s) + 1,
                aux, lstrlenW(s),
                NULL, NULL
            );
            result = aux;
            delete[] aux;
            LocalFree(s);
        }
#else
        result = strerror(errno);
#endif

        return result;
    }


    // Returns true on success, or false if there was an error
    bool SocketUtils::SetSocketBlockingEnabled(int fd, bool blocking) {
        if (fd < 0) return false;

#ifdef _WIN32
        unsigned long mode = blocking ? 0 : 1;
        return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return false;
        flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
        return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif

    }

}
