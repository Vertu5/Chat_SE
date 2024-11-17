// signal_handler.cpp
#include "signal_handler.hpp"
#include <iostream>

namespace Chat {

    void signalHandler(int signal) {
        if (signal == SIGINT) {
            if (g_opts.isManual && g_sharedMemory != nullptr) {
                g_sigintReceived = 1;
            } else {
                g_shutdown = 1;
                g_running = 0;
            }
        }
        else if (signal == SIGPIPE) {
            g_sigpipeReceived = 1;
        }
        else if (signal == SIGUSR1) {
            g_displayPendingMessages = 1;
        }
    }

    void setupSignalHandlers() {
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0; // Retiré SA_RESTART

        // Configuration des signaux
        if (sigaction(SIGINT, &sa, NULL) == -1) {
            std::cerr << "Erreur configuration SIGINT" << std::endl;
            exit(SYSTEM_ERROR);
        }
        if (sigaction(SIGPIPE, &sa, NULL) == -1) {
            std::cerr << "Erreur configuration SIGPIPE" << std::endl;
            exit(SYSTEM_ERROR);
        }
        if (sigaction(SIGUSR1, &sa, NULL) == -1) {
            std::cerr << "Erreur configuration SIGUSR1" << std::endl;
            exit(SYSTEM_ERROR);
        }
    }


}