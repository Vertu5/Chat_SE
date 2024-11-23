#include "normal_mode.hpp"
#include "pipes.hpp"
#include <sys/select.h>

namespace Chat {

NormalMode::NormalMode(const ProgramOptions& opts, 
                      const std::string& sendPipe, 
                      const std::string& receivePipe)
    : ChatMode(opts, sendPipe, receivePipe) {}

void NormalMode::runParentProcess() {
    auto result = Pipes::openWritePipe(sendPipe, opts);
    int writeFd = result.fd;

    // Passage en mode non-bloquant pour l'écriture
    if (!Pipes::setupPipeFlags(writeFd, true)) {
        close(writeFd);
        exit(SYSTEM_ERROR);
    }

    std::string line;
    while (!g_shutdown && g_running) {
        // Gestion des signaux
        if (g_sigintReceived) {
            if (g_signalStage == INIT) {
                std::cerr << "\nInterruption pendant l'initialisation..." << std::endl;
                cleanupPipes(sendPipe, receivePipe);
                exit(SIGINT_EXIT);
            } else {
                g_shutdown = 1;
                g_running = 0;
            }
            g_sigintReceived = 0;
        }
        if (g_sigpipeReceived) {          
            g_shutdown = 1;
            g_running = 0;
            g_sigpipeReceived = 0;
            break;
        }
        
        if (!std::cin.good() || g_shutdown) {
            g_running = 0;
            break;
        }
        
        std::getline(std::cin, line);
        
        if (std::cin.eof() || line == "exit" || g_shutdown) {
            g_running = 0;
            break;
        }

        if (!line.empty()) {
            Message msg;
            strncpy(msg.from, opts.user.c_str(), MAX_PSEUDO_LENGTH);
            strncpy(msg.to, opts.dest.c_str(), MAX_PSEUDO_LENGTH);
            strncpy(msg.content, line.c_str(), sizeof(msg.content) - 1);

            ssize_t bytesWritten = write(writeFd, &msg, sizeof(Message));
            if (bytesWritten <= 0 && !Pipes::isNormalError(bytesWritten, errno)) {
                std::cerr << "Erreur d'écriture" << std::endl;
                g_running = 0;
                break;
            }

            if (!g_shutdown && g_running && !opts.isBot) {
                displayMessage(msg, false);
            }
        }
    }

    close(writeFd);
}

void NormalMode::runChildProcess() {
    signal(SIGINT, SIG_IGN);

    int readFd = Pipes::initializeReadPipe(receivePipe, true);
    
    fd_set readfds;
    struct timeval tv;
    int noDataCount = 0;

    while (!g_shutdown && g_running) {
        FD_ZERO(&readfds);
        FD_SET(readFd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        int ret = select(readFd + 1, &readfds, NULL, NULL, &tv);
        
        if (g_shutdown || !g_running) break;
        
        if (ret > 0) {
            Message msg;
            ssize_t bytesRead = read(readFd, &msg, sizeof(Message));
            
            if (bytesRead <= 0) {
                if (!Pipes::isNormalError(bytesRead, errno)) {
                    noDataCount++;
                    if (noDataCount > 2) {
                        if (opts.isJoli) {
                            std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                        }
                        g_running = 0;
                        g_shutdown = 1;
                        kill(getppid(), SIGPIPE);
                        break;
                    }
                }
                continue;
            }

            noDataCount = 0;
            if (!g_shutdown && g_running) {
                displayMessage(msg, opts.isBot);
            }
        }
        else if (ret == 0 && !isPipeValid(receivePipe)) {
            if (opts.isJoli) {
                std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
            }
            g_running = 0;
            g_shutdown = 1;
            kill(getppid(), SIGPIPE);
            break;
        }
    }

    close(readFd);
    exit(SUCCESS);
}

}  // namespace Chat