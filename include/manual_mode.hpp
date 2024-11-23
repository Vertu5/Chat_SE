/**
 * @file manual_mode.hpp 
 * @brief Définition du mode manuel du chat (utilisateur)
 */

#pragma once
#include "chat_mode.hpp"
#include "shared_memory.hpp"
#include <memory>

namespace Chat {
    class ManualMode : public ChatMode {
    protected:
        std::unique_ptr<SharedMemory> sharedMem;  // Pointeur unique pour la mémoire partagée
        bool displayPendingMessages;  // Indicateur pour afficher les messages en attente

    public:
        /**
         * @brief Constructeur de la classe ManualMode
         * @param opts Options du programme
         * @param sendPipe Nom du pipe pour l'envoi des messages
         * @param receivePipe Nom du pipe pour la réception des messages
         */
        ManualMode(const ProgramOptions& opts,
                   const std::string& sendPipe,
                   const std::string& receivePipe);

        /**
         * @brief Destructeur par défaut
         * Le unique_ptr s'occupera du nettoyage
         */
        ~ManualMode() = default;

        /**
         * @brief Fonction pour exécuter le processus parent
         */
        void runParentProcess() override;

        /**
         * @brief Fonction pour exécuter le processus enfant
         */
        void runChildProcess() override;
    };
}