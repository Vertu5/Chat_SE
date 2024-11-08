#include "shared_memory.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>     // for close, ftruncate
#include <sys/mman.h>   // for mmap, munmap
#include <fcntl.h>      // for shm_open
#include <sys/stat.h>   // for mode_t

namespace Chat {

SharedMemory::SharedMemory(const std::string& userName) 
    : shmName("/chat_" + userName), shmFd(-1), buffer(nullptr) {
    
    // Preventive cleanup
    shm_unlink(shmName.c_str());
    
    shmFd = shm_open(shmName.c_str(), O_CREAT | O_RDWR, 0600);
    if (shmFd == -1) {
        throw std::runtime_error("Erreur création mémoire partagée");
    }

    if (ftruncate(shmFd, sizeof(SharedMemoryBuffer)) == -1) {
        close(shmFd);
        shm_unlink(shmName.c_str());
        throw std::runtime_error("Erreur initialisation taille mémoire partagée");
    }

    buffer = static_cast<SharedMemoryBuffer*>(
        mmap(nullptr, sizeof(SharedMemoryBuffer), 
             PROT_READ | PROT_WRITE, MAP_SHARED, 
             shmFd, 0)
    );

    if (buffer == MAP_FAILED) {
        close(shmFd);
        shm_unlink(shmName.c_str());
        throw std::runtime_error("Erreur mapping mémoire partagée");
    }

    new (buffer) SharedMemoryBuffer();
}

SharedMemory::~SharedMemory() {
    cleanup();
}

bool SharedMemory::addMessage(const std::string& msg) {
    if (buffer->messageCount >= SharedMemoryBuffer::MAX_MESSAGES) {
        return false;
    }

    if (buffer->totalBytes + msg.length() > 4096) {
        displayMessages();
        buffer->messageCount = 0;
        buffer->totalBytes = 0;
    }

    PendingMessage& pending = buffer->messages[buffer->messageCount];
    strncpy(pending.content, msg.c_str(), sizeof(pending.content) - 1);
    pending.size = msg.length();
    pending.read = false;
    
    buffer->messageCount++;
    buffer->totalBytes += msg.length();
    
    return true;
}

void SharedMemory::displayMessages() {
    for (size_t i = 0; i < buffer->messageCount; i++) {
        if (!buffer->messages[i].read) {
            std::cout << buffer->messages[i].content << std::endl;
            buffer->messages[i].read = true;
        }
    }
}

bool SharedMemory::shouldDisplay() const {
    return buffer->totalBytes > 4096;
}

void SharedMemory::cleanup() {
    if (buffer != MAP_FAILED && buffer != nullptr) {
        munmap(buffer, sizeof(SharedMemoryBuffer));
    }
    if (shmFd != -1) {
        close(shmFd);
        shm_unlink(shmName.c_str());
    }
}

}

