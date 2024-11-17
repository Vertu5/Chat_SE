/**
 * @file signal_handler.hpp 
 * @brief Définition de la gestion des signaux pour le chat
 */

#pragma once
#include <signal.h>
#include "chat.hpp"

namespace Chat {
    void setupSignalHandlers();
    void signalHandler(int signal);
}
