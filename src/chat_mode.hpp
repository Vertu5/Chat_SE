/**
 * @file chat_mode.hpp 
 * @brief Définition de la gestion des modes bot du chat
 */

#pragma once
#include "chat.hpp"

namespace Chat {
    /**
     * @brief Classe abstraite définissant l'interface de base pour les différents modes de chat
     * 
     * Cette classe fournit une interface commune pour tous les modes de chat (normal, 
     * manuel, bot). Elle gère les aspects fondamentaux de la communication entre 
     * deux utilisateurs via des pipes nommés.
     */
    class ChatMode {
    protected:
        /** @brief Options de configuration du programme */
        const ProgramOptions& opts;
        
        /** @brief Chemin du pipe pour l'envoi des messages */
        const std::string sendPipe;
        
        /** @brief Chemin du pipe pour la réception des messages */
        const std::string receivePipe;

    public:
        /**
         * @brief Constructeur
         * @param options Configuration du programme
         * @param send Chemin du pipe d'envoi
         * @param receive Chemin du pipe de réception
         */
        ChatMode(const ProgramOptions& options, 
                const std::string& send, 
                const std::string& receive) 
            : opts(options), sendPipe(send), receivePipe(receive) {}
        
        /** @brief Destructeur virtuel pour permettre l'héritage */
        virtual ~ChatMode() = default;

        /**
         * @brief Exécute la logique du processus parent
         * 
         * Le processus parent est responsable de :
         * - Lire les messages depuis stdin
         * - Les envoyer via le pipe d'envoi
         * - Gérer les signaux SIGINT et SIGPIPE
         */
        virtual void runParentProcess() = 0;

        /**
         * @brief Exécute la logique du processus enfant
         * 
         * Le processus enfant est responsable de :
         * - Lire les messages depuis le pipe de réception
         * - Les afficher selon le mode configuré
         * - Ignorer le signal SIGINT
         */
        virtual void runChildProcess() = 0;
        
        /**
         * @brief Affiche un message selon le mode configuré
         * @param msg Message à afficher
         * @param isBot True si en mode bot (pas de formatage)
         * 
         * En mode normal : affiche avec soulignement du pseudo
         * En mode bot : affiche sans formatage
         */
        static void displayMessage(const Message& msg, bool isBot);
    };
}