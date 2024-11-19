// manual_mode.cpp
#include "manual_mode.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

namespace Chat {

ManualMode::ManualMode(const ProgramOptions& opts,
                      const std::string& sendPipe,
                      const std::string& receivePipe)
    : ChatMode(opts, sendPipe, receivePipe) {
    sharedMem = std::make_unique<SharedMemory>(sizeof(SharedMemoryBuffer));
    g_sharedMemory = sharedMem.get();  // Si vous devez garder cette référence globale
}

void ManualMode::runParentProcess() {
    int writeFd;
    while (!g_shutdown) {
        writeFd = open(sendPipe.c_str(), O_WRONLY | O_NONBLOCK);
        if (writeFd != -1) break;
        if (errno == ENXIO) {
            usleep(500000);
        } else {
            std::cerr << "Erreur ouverture pipe écriture" << std::endl;
            exit(PIPE_ERROR);
        }
    }

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
        if (g_sigintReceived) {
            if (sharedMem) {
                std::cout << "\n";
                sharedMem->displayMessages();
            }
            // gestion du mode Bot
            if (g_opts.isBot) {
                g_shutdown = 1;
                g_running = 0;
                break;
            }
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
            // affichage de message en attente avant de quitter
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
            if (bytesWritten <= 0) {
                if (errno == EINTR) continue;
                std::cerr << "Erreur d'écriture" << std::endl;
                g_running = 0;
                break;
            }

            // Utiliser displayMessage avec le flag isBot
            if (!opts.isBot) {
                if (opts.isJoli) {
                    // Efface la ligne courante pour le message entrant
                    //std::cout << "\r" << std::string(80, ' ') << "\r";
                }
                displayMessage(msg, opts.isBot);
            }

            if (sharedMem) {
                if (opts.isJoli) {
                    // Efface la ligne courante pour le message entrant
                    //std::cout << "\r" << std::string(80, ' ') << "\r";
                }
                sharedMem->displayMessages();
            }
        }
    }

    close(writeFd);
}

void ManualMode::runChildProcess() {
    signal(SIGINT, SIG_IGN);

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
            std::cout << '\a' << std::flush;

            // Construction du message avec l'indentation appropriée
            std::string formattedMsg;
            if (opts.isJoli && msg.from != g_opts.user && msg.to == g_opts.user) {
                formattedMsg = std::string(40, ' ');
            }
            
            formattedMsg += opts.isBot ? 
                "[" + std::string(msg.from) + "] " + msg.content :
                "\x1B[4m[" + std::string(msg.from) + "]\x1B[0m " + msg.content;

            sharedMem->addMessage(formattedMsg);

            if (sharedMem->shouldDisplay()) {
                kill(getppid(), SIGUSR1);
            }
        } else if (bytesRead == 0) {
            if (communicationStarted) {
                if (opts.isJoli) {
                    std::cout << "\nL'autre utilisateur s'est déconnecté." << std::endl;
                }

                g_running = 0;
                g_shutdown = 1;
                kill(getppid(), SIGPIPE);  // Ajout de cette ligne pour notifier le parent pour qu il se deconnecte aussi
                break;
            }
            continue;
        } else {
            if (errno == EINTR) continue;
            std::cerr << "Erreur de lecture du pipe" << std::endl;
            g_running = 0;
            g_shutdown = 1;
            kill(getppid(), SIGPIPE);  // Ajout de cette ligne pour notifier le parent
            break;
        }
    }

    close(readFd);
    exit(SUCCESS);
}
}