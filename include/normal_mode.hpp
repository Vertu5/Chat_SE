/**
 * @file normal_mode.hpp 
 * @brief Définition du mode normal du chat (utilisateur)
 * 
 * Ce fichier contient la déclaration de la classe NormalMode, qui hérite de ChatMode.
 * Cette classe représente le mode de fonctionnement normal pour un utilisateur du chat.
 */

#pragma once
#include "chat_mode.hpp"

namespace Chat {
    /**
     * @class NormalMode
     * @brief Classe représentant le mode normal du chat.
     * 
     * La classe NormalMode hérite de ChatMode et implémente les méthodes nécessaires
     * pour gérer les processus parent et enfant dans le contexte d'un chat.
     */
    class NormalMode : public ChatMode {
    public:
        /**
         * @brief Constructeur de la classe NormalMode.
         * 
         * @param opts Options du programme.
         * @param sendPipe Nom du tube pour l'envoi des messages.
         * @param receivePipe Nom du tube pour la réception des messages.
         */
        NormalMode(const ProgramOptions& opts, 
                  const std::string& sendPipe, 
                  const std::string& receivePipe);

        /**
         * @brief Méthode pour exécuter le processus parent.
         * 
         * Cette méthode est appelée pour gérer le comportement du processus parent.
         */
        void runParentProcess() override;

        /**
         * @brief Méthode pour exécuter le processus enfant.
         * 
         * Cette méthode est appelée pour gérer le comportement du processus enfant.
         */
        void runChildProcess() override;
    };
}


