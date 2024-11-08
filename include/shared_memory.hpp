#pragma once
#include <sys/mman.h>
#include <fcntl.h>
#include <string>

namespace Chat {
    struct PendingMessage {
        char content[1024];
        bool read;
        size_t size;
    };

    struct SharedMemoryBuffer {
        static const size_t MAX_MESSAGES = 100;
        PendingMessage messages[MAX_MESSAGES];
        size_t messageCount;
        size_t totalBytes;
        
        SharedMemoryBuffer() : messageCount(0), totalBytes(0) {}
    };

    class SharedMemory {
    private:
        std::string shmName;
        int shmFd;
        SharedMemoryBuffer* buffer;
        
    public:
        SharedMemory(const std::string& userName);
        ~SharedMemory();
        
        bool addMessage(const std::string& msg);
        void displayMessages();
        bool shouldDisplay() const;
        void cleanup();
    };
}

