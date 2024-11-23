#include "pipes.hpp"
#include <unistd.h>
#include <errno.h>

namespace Chat {
namespace Pipes {

PipeOpenResult openWritePipe(const std::string& pipePath, const ProgramOptions& opts) {
    PipeOpenResult result = {-1, false};
    
    result.fd = open(pipePath.c_str(), O_WRONLY | O_NONBLOCK);
    
    if (result.fd == -1 && errno != ENXIO) {
        std::cerr << "Erreur ouverture pipe écriture" << std::endl;
        exit(PIPE_ERROR);
    }

    if (result.fd == -1) {
        if (opts.isJoli) {
            std::cout << "En attente du destinataire..." << std::endl;
        }
        result.waited = true;
        
        while (!g_shutdown && result.fd == -1) {
            result.fd = open(pipePath.c_str(), O_WRONLY);
            if (result.fd == -1) {
                usleep(500000);
            }
        }

        if (g_shutdown) {
            if (opts.isJoli) {
                std::cerr << "Interruption pendant l'attente" << std::endl;
            }
            close(result.fd);
            exit(SYSTEM_ERROR);
        }
        if (opts.isJoli) {
            std::cout << "Connexion établie!" << std::endl;
            std::cout << "\n";
        }
    } else {
        if (opts.isJoli) {
            std::cout << "Connexion établie!" << std::endl;
            std::cout << "\n";
        }
    }
    
    return result;
}

bool setupPipeFlags(int fd, bool nonBlocking) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) return false;
    
    if (nonBlocking) 
        flags |= O_NONBLOCK;
    else 
        flags &= ~O_NONBLOCK;
    
    return fcntl(fd, F_SETFL, flags) != -1;
}

int initializeReadPipe(const std::string& pipePath, bool nonBlocking) {
    int flags = O_RDONLY;
    if (nonBlocking) flags |= O_NONBLOCK;
    
    int readFd = open(pipePath.c_str(), flags);
    if (readFd == -1) {
        std::cerr << "Erreur ouverture pipe lecture" << std::endl;
        exit(PIPE_ERROR);
    }
    return readFd;
}

bool isNormalError(int bytesProcessed, int err) {
    return (bytesProcessed <= 0 && 
            (err == EAGAIN || err == EWOULDBLOCK || err == EINTR));
}

} // namespace Pipes
} // namespace Chat