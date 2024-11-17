#pragma once
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <memory>
#include <stdexcept>

namespace Chat {
    struct PendingMessage {
        char content[1024];
        bool read;
        size_t size;
    };

    struct SharedMemoryBuffer {
        static const size_t MAX_MESSAGES = 1000;
        PendingMessage messages[MAX_MESSAGES];
        std::atomic<size_t> messageCount;
        std::atomic<size_t> totalBytes;
        
        SharedMemoryBuffer() : messageCount(0), totalBytes(0) {}
    };

    class SharedMemory {
    private:
        void* buffer;
        size_t size;
        
    public:
        explicit SharedMemory(size_t requiredSize) : size(requiredSize) {
            buffer = mmap(nullptr, size, 
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_SHARED, 
                         -1, 0);
            
            if (buffer == MAP_FAILED) {
                throw std::runtime_error("Échec allocation mémoire partagée");
            }
            
            // Initialize the buffer
            auto* sharedBuffer = static_cast<SharedMemoryBuffer*>(buffer);
            new (sharedBuffer) SharedMemoryBuffer();
        }
        
        ~SharedMemory() {
            if (buffer != MAP_FAILED) {
                munmap(buffer, size);
            }
        }
        
        // Prevent copying
        SharedMemory(const SharedMemory&) = delete;
        SharedMemory& operator=(const SharedMemory&) = delete;
        
        // Non-const version
        SharedMemoryBuffer* get() { 
            return static_cast<SharedMemoryBuffer*>(buffer); 
        }
        
        // Const version
        const SharedMemoryBuffer* get() const { 
            return static_cast<const SharedMemoryBuffer*>(buffer); 
        }
        
        bool addMessage(const std::string& msg);
        void displayMessages();
        bool shouldDisplay() const;
    };
}