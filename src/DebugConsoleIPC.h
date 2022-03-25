#ifndef _____DEBUG_CONSOLE__IPC__H__
#define _____DEBUG_CONSOLE__IPC__H__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/PlatformLowLatencyQueueIPC.h>

#include <stdarg.h> //va_start

namespace aRibeiro {

    class DebugConsoleIPC {

        std::vector<char> char_buffer;
        uint32_t mode;
    public:
        PlatformLowLatencyQueueIPC queue;

        DebugConsoleIPC(uint32_t mode = PlatformQueueIPC_WRITE) :
            queue("debug_console", mode, 1024, sizeof(char) * 64) {

            this->mode = mode;

        }

        void runReadLoop() {
            if ((mode & PlatformQueueIPC_READ) == 0)
                return;

            ::printf("read main loop\n");
            fflush(stdin);
            ObjectBuffer buffer;
            while (queue.read(&buffer)) {
                //::printf("data received: %i", buffer.size);
                ::printf("%s", (char*)buffer.data);
                fflush(stdin);
            }
        }

        const char* printf(const char* format, ...) {

            if ((mode & PlatformQueueIPC_WRITE) == 0)
                return "";

            va_list args;

            va_start(args, format);
            char_buffer.resize(vsnprintf(NULL, 0, format, args) + 1);
            va_end(args);

            va_start(args, format);
            int len = vsnprintf(&char_buffer[0], char_buffer.size(), format, args);
            va_end(args);

            if (char_buffer.size() == 0)
                return "";

            queue.write((uint8_t*)&char_buffer[0], (uint32_t)char_buffer.size(), false);

            return &char_buffer[0];
        }


    };

}

#endif