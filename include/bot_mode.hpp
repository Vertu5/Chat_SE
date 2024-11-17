/**
 * @file bot_mode.hpp 
 * @brief Définition de la classe pour le mode bot du chat
 */

#pragma once
#include "normal_mode.hpp"

namespace Chat {
    /**
     * @brief Classe gérant le mode bot du chat
     * 
     * @details Cette classe hérite de NormalMode et modifie le comportement 
     * d'affichage pour :
     * - Désactiver le formatage du texte (pas de soulignement)
     * - Ne pas afficher les messages envoyés par l'utilisateur
     * Le mode bot est activé avec l'option --bot
     */
    class BotMode : public NormalMode {
    public:
        /**
         * @brief Constructeur de BotMode
         * 
         * @param opts Configuration du programme incluant les pseudonymes
         * @param sendPipe Chemin du pipe pour l'envoi des messages (/tmp/user-dest.chat)
         * @param receivePipe Chemin du pipe pour la réception (/tmp/dest-user.chat)
         */
        BotMode(const ProgramOptions& opts,
                const std::string& sendPipe,
                const std::string& receivePipe);
    };
}