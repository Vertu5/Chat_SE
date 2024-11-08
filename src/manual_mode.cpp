#include "manual_mode.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

namespace Chat {

ManualMode::ManualMode(const ProgramOptions& opts,
                      const std::string& sendPipe,
                      const std::string& receivePipe)
    : ChatMode(opts, sendPipe, receivePipe) {
    sharedMem = new SharedMemory(opts.user);
    g_sharedMemory = sharedMem;
}

ManualMode::~ManualMode() {
    g_sharedMemory = nullptr;
    delete sharedMem;
}

void ManualMode::runParentProcess() {
    int writeFd;
    while (!g_shutdown) {
        writeFd = open(sendPipe.c_str(), O_WRONLY | O_NONBLOCK);
        if (writeFd != -1) {
            break;
        }
        if (errno == ENXIO) {
            // No reader yet, sleep and retry
            usleep(500000);
        } else {
            std::cerr << "Erreur ouverture pipe écriture" << std::endl;
            exit(PIPE_ERROR);
        }
    }

    // Set writeFd to blocking mode
    int flags = fcntl(writeFd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Erreur récupération flags" << std::endl;
        close(writeFd);
        exit(SYSTEM_ERROR);
    }
    flags &= ~O_NONBLOCK;
    if (fcntl(writeFd, F_SETFL, flags) == -1) {
        std::cerr << "Erreur passage mode bloquant" << std::endl;
        close(writeFd);
        exit(SYSTEM_ERROR);
    }

    std::string line;
    while (!g_shutdown && g_running) {
        // Check for signals
        if (g_sigintReceived) {
            // Display pending messages on SIGINT
            if (sharedMem) {
                std::cout << "\n";
                sharedMem->displayMessages();
                std::cout << "Message: " << std::flush;
            }
            g_sigintReceived = 0;
        }

        if (g_displayPendingMessages) {
            // Display messages when buffer exceeds 4096 bytes
            if (sharedMem) {
                std::cout << "\n";
                sharedMem->displayMessages();
                std::cout << "Message: " << std::flush;
            }
            g_displayPendingMessages = 0;
        }

        if (g_sigpipeReceived) {
            std::cout << "\nConnexion perdue avec l'autre utilisateur" << std::endl;
            g_shutdown = 1;
            g_running = 0;
            break;
        }

        std::cout << "Message: " << std::flush;

        if (!std::cin.good() || g_shutdown) {
            g_running = 0;
            break;
        }

        std::getline(std::cin, line);

        if (std::cin.eof() || line == "exit" || g_shutdown) {
            g_running = 0;
            break;
        }

        Message msg;
        strncpy(msg.from, opts.user.c_str(), MAX_PSEUDO_LENGTH);
        strncpy(msg.to, opts.dest.c_str(), MAX_PSEUDO_LENGTH);
        strncpy(msg.content, line.c_str(), sizeof(msg.content) - 1);

        ssize_t bytesWritten = write(writeFd, &msg, sizeof(Message));
        if (bytesWritten <= 0) {
            if (errno == EINTR) {
                continue;  // Interrupted, retry
            } else {
                std::cerr << "Erreur d'écriture" << std::endl;
                g_running = 0;
                break;
            }
        }

        // Display our message
        displayMessage(msg, false);

        // After sending, display any pending messages
        if (sharedMem) {
            sharedMem->displayMessages();
        }
    }

    close(writeFd);
}

void ManualMode::runChildProcess() {
    signal(SIGINT, SIG_IGN);  // Child ignores SIGINT

    // Open the receive pipe in blocking mode
    int readFd = open(receivePipe.c_str(), O_RDONLY);
    if (readFd == -1) {
        std::cerr << "Erreur ouverture pipe lecture" << std::endl;
        exit(PIPE_ERROR);
    }

    bool communicationStarted = false;

    while (!g_shutdown && g_running) {
        Message msg;
        ssize_t bytesRead = read(readFd, &msg, sizeof(Message));

        if (bytesRead > 0) {
            communicationStarted = true;

            // Emit a beep on reception
            std::cout << '\a' << std::flush;

            // Format and store the message
            std::string formattedMsg = "\x1B[4m[" + std::string(msg.from) + "]\x1B[0m " + msg.content;
            sharedMem->addMessage(formattedMsg);

            // Signal parent if necessary
            if (sharedMem->shouldDisplay()) {
                kill(getppid(), SIGUSR1);
            }
        } else if (bytesRead == 0) {
            if (communicationStarted) {
                // The write end of the pipe has been closed
                std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                g_running = 0;
                break;
            } else {
                // Haven't started communication yet, continue waiting
                continue;
            }
        } else {
            if (errno == EINTR) {
                continue;  // Interrupted, retry
            } else {
                std::cerr << "Erreur de lecture du pipe" << std::endl;
                g_running = 0;
                break;
            }
        }
    }

    close(readFd);
    exit(SUCCESS);
}

}

