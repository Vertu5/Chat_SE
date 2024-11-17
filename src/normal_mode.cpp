#include "normal_mode.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

namespace Chat {

NormalMode::NormalMode(const ProgramOptions& opts, 
                       const std::string& sendPipe, 
                       const std::string& receivePipe)
    : ChatMode(opts, sendPipe, receivePipe) {}

void NormalMode::runParentProcess() {
    int writeFd = open(sendPipe.c_str(), O_WRONLY);
    if (writeFd == -1) {
        std::cout << "En attente du destinataire..." << std::endl;
        
        while (!g_shutdown && writeFd == -1) {
            writeFd = open(sendPipe.c_str(), O_WRONLY);
            if (writeFd == -1) {
                usleep(500000);
            }
        }
        
        if (g_shutdown) {
            std::cerr << "Interruption pendant l'attente" << std::endl;
            close(writeFd);
            exit(SYSTEM_ERROR);
        }
    }

    int flags = fcntl(writeFd, F_GETFL);
    if (flags == -1) {
        std::cerr << "Erreur récupération flags" << std::endl;
        close(writeFd);
        exit(SYSTEM_ERROR);
    }
    
    if (fcntl(writeFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Erreur passage mode non-bloquant" << std::endl;
        close(writeFd);
        exit(SYSTEM_ERROR);
    }

    std::string line;
    while (!g_shutdown && g_running) {
        // Check for signals
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
            std::cout << "\nConnexion perdue avec l'autre utilisateur" << std::endl;
            g_shutdown = 1;
            g_running = 0;
            g_sigpipeReceived = 0;
            break;
        }

        //std::cout << "Message: " << std::flush;
        
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

            if (!g_shutdown && write(writeFd, &msg, sizeof(Message)) <= 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cerr << "Erreur d'écriture" << std::endl;
                    g_running = 0;
                    break;
                }
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

    int readFd = open(receivePipe.c_str(), O_RDONLY | O_NONBLOCK);
    if (readFd == -1) {
        std::cerr << "Erreur ouverture pipe lecture" << std::endl;
        exit(PIPE_ERROR);
    }

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
                if (bytesRead == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
                    noDataCount++;
                    if (noDataCount > 2) {
                        std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                        g_running = 0;
                        g_shutdown = 1;  // Ajout de cette ligne
                        kill(getppid(), SIGPIPE);  // Signal au parent
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
        else if (ret == 0) {
            if (!isPipeValid(receivePipe)) {
                std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                g_running = 0;
                g_shutdown = 1;  // Ajout de cette ligne
                kill(getppid(), SIGPIPE);  // Signal au parent
                break;
            }
        }
    }

    close(readFd);
    exit(SUCCESS);
}
}

