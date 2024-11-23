/**
 * @file signal_handler.hpp 
 * @brief Définition de la gestion des signaux pour le chat
 */

#pragma once
#include <signal.h>
#include "chat.hpp"

namespace Chat {
    /**
     * @brief Configure les gestionnaires de signaux pour l'application de chat.
     * 
     * Cette fonction configure les gestionnaires de signaux nécessaires pour
     * gérer proprement les interruptions et autres signaux du système.
     */
    void setupSignalHandlers();

    /**
     * @brief Gestionnaire de signaux pour l'application de chat.
     * 
     * Cette fonction est appelée lorsqu'un signal spécifique est reçu par
     * l'application. Elle permet de gérer les signaux de manière appropriée.
     * 
     * @param signal Le signal reçu par l'application.
     */
    void signalHandler(int signal);
}
