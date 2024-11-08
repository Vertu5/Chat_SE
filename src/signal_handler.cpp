#include "signal_handler.hpp"
#include "chat.hpp"
#include <iostream>

namespace Chat {

void setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
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

void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_sigintReceived = 1;
    } else if (signal == SIGPIPE) {
        g_sigpipeReceived = 1;
    } else if (signal == SIGUSR1) {
        g_displayPendingMessages = 1;
    }
}

}

