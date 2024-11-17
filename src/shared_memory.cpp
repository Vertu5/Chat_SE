#include "shared_memory.hpp"
#include <iostream>
#include <cstring>

namespace Chat {

bool SharedMemory::addMessage(const std::string& msg) {
    auto* sharedBuffer = get();
    if (sharedBuffer->messageCount >= SharedMemoryBuffer::MAX_MESSAGES) {
        return false;
    }

    size_t currentBytes = sharedBuffer->totalBytes.load();
    if (currentBytes + msg.length() > 4096) {
        displayMessages();
        sharedBuffer->messageCount.store(0);
        sharedBuffer->totalBytes.store(0);
    }

    PendingMessage& pending = sharedBuffer->messages[sharedBuffer->messageCount];
    strncpy(pending.content, msg.c_str(), sizeof(pending.content) - 1);
    pending.content[sizeof(pending.content) - 1] = '\0';
    pending.size = msg.length();
    pending.read = false;
    
    sharedBuffer->messageCount++;
    sharedBuffer->totalBytes += msg.length();
    
    return true;
}

void SharedMemory::displayMessages() {
    auto* sharedBuffer = get();
    for (size_t i = 0; i < sharedBuffer->messageCount; i++) {
        if (!sharedBuffer->messages[i].read) {
            std::cout << sharedBuffer->messages[i].content << std::endl;
            sharedBuffer->messages[i].read = true;
        }
    }
}

bool SharedMemory::shouldDisplay() const {
    return get()->totalBytes >= 4096;
}

}