#include "manual_mode.hpp"
#include "pipes.hpp"

namespace Chat {

ManualMode::ManualMode(const ProgramOptions& opts,
                      const std::string& sendPipe,
                      const std::string& receivePipe)
    : ChatMode(opts, sendPipe, receivePipe) {
    sharedMem = std::make_unique<SharedMemory>(sizeof(SharedMemoryBuffer));
    g_sharedMemory = sharedMem.get();
}

void ManualMode::runParentProcess() {
    auto result = Pipes::openWritePipe(sendPipe, opts);
    int writeFd = result.fd;
    
    // Passage en mode bloquant pour le mode manuel
    if (!Pipes::setupPipeFlags(writeFd, false)) {
        close(writeFd);
        exit(SYSTEM_ERROR);
    }

    std::string line;
    while (!g_shutdown && g_running) {
        if (g_sigintReceived) {
            if (sharedMem) {
                //std::cout << "\n";
                sharedMem->displayMessages();
            }
            // if (g_opts.isBot) {
            //     g_shutdown = 1;
            //     g_running = 0;
            //     break;
            // }
            g_sigintReceived = 0;
            std::cin.clear();
            continue;
        }

        if (g_displayPendingMessages) {
            if (sharedMem) {
                std::cout << "\n";
                if (opts.isJoli) {
                    std::cout << "Affichage de messages en attente:" << std::endl;
                }
                sharedMem->displayMessages();
            }
            g_displayPendingMessages = 0;
        }

        if (g_sigpipeReceived) {
            if (sharedMem) {
                sharedMem->displayMessages();
            }
            g_shutdown = 1;
            g_running = 0;
            break;
        }

        std::getline(std::cin, line);

        if (line == "exit" || g_shutdown) {
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

            if (!opts.isBot) {
                displayMessage(msg, opts.isBot);
            }

            if (sharedMem) {
                sharedMem->displayMessages();
            }
        }
    }

    close(writeFd);
}

void ManualMode::runChildProcess() {
    signal(SIGINT, SIG_IGN);

    int readFd = Pipes::initializeReadPipe(receivePipe, false);
    bool communicationStarted = false;

    while (!g_shutdown && g_running) {
        Message msg;
        ssize_t bytesRead = read(readFd, &msg, sizeof(Message));

        if (bytesRead > 0) {
            communicationStarted = true;
            std::cout << '\a' << std::flush;

            std::string formattedMsg;
            if (opts.isJoli && msg.from != g_opts.user && msg.to == g_opts.user) {
                formattedMsg = std::string(40, ' ');
            }
            
            formattedMsg += opts.isBot ? 
                "[" + std::string(msg.from) + "] " + msg.content :
                "\x1B[4m[" + std::string(msg.from) + "]\x1B[0m " + msg.content;

            if (opts.isBot && opts.isManual) {
                // En mode bot + manuel, afficher directement le message
                std::cout << formattedMsg << std::endl;
            } else {
                // Sinon, utiliser le système de messages en attente
                sharedMem->addMessage(formattedMsg);
                if (sharedMem->shouldDisplay()) {
                    kill(getppid(), SIGUSR1);
                }
            }
        }else if (bytesRead == 0) {
            if (communicationStarted) {
                if (opts.isJoli) {
                    std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                }

                g_running = 0;
                g_shutdown = 1;
                kill(getppid(), SIGPIPE);  
                break;
            }
            continue;
        } else {
            if (errno == EINTR) continue;
            std::cerr << "Erreur de lecture du pipe" << std::endl;
            g_running = 0;
            g_shutdown = 1;
            kill(getppid(), SIGPIPE);  
            break;
        }
    }

    close(readFd);
    exit(SUCCESS);
}
    
} // namespace Chat