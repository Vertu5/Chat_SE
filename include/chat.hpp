/**
 * @file chat.hpp 
 * @brief Définition des variables globales et des fonctions utilitaires
 */

#pragma once
#include <string>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <unistd.h> 
#include "shared_memory.hpp"

namespace Chat {
    // Constantes
    const int MAX_PSEUDO_LENGTH = 30; // Longueur maximale du pseudo
    const std::string PIPE_PATH = "/tmp/"; // Chemin de base pour les pipes
    const std::string PIPE_EXT = ".chat"; // Extension des pipes

    // Codes de retour
    enum ReturnCodes {
        SUCCESS = 0, // Succès
        MISSING_ARGS = 1, 
        PSEUDO_TOO_LONG = 2, 
        INVALID_CHARS = 3,
        SIGINT_EXIT = 4, 
        PIPE_ERROR = 5, 
        SYSTEM_ERROR = 6, 
        RESOURCE_ERROR = 7, 
        DISCONNECTED = 8,
        STDIN_CLOSED = 9 
    };

    // Étapes du signal
    enum SignalStage {
        INIT, 
        RUNNING 
    };

    // Structure pour les messages
    struct Message {
        char from[MAX_PSEUDO_LENGTH + 1]; // Expéditeur
        char to[MAX_PSEUDO_LENGTH + 1]; // Destinataire
        char content[1024]; // Contenu du message

        Message() {
            memset(from, 0, sizeof(from)); // Initialiser 'from' à zéro
            memset(to, 0, sizeof(to)); // Initialiser 'to' à zéro
            memset(content, 0, sizeof(content)); // Initialiser 'content' à zéro
        }
    };

    // Structure pour les options du programme
    struct ProgramOptions {
        std::string user; // Utilisateur
        std::string dest; // Destinataire
        bool isBot = false;
        bool isManual = false; 
        bool isJoli = false; 
    };

    // Variables globales
    extern std::string g_sendPipePath; // Chemin du pipe d'envoi
    extern std::string g_receivePipePath; // Chemin du pipe de réception
    extern bool g_pipesOpened; // Les pipes sont-ils ouverts ?
    extern volatile sig_atomic_t g_running; // Le programme est-il en cours d'exécution ?
    extern volatile sig_atomic_t g_shutdown; // Le programme doit-il s'arrêter ?
    extern SignalStage g_signalStage; // Étape actuelle du signal
    extern ProgramOptions g_opts; // Options du programme
    extern SharedMemory* g_sharedMemory; // Mémoire partagée

    // Drapeaux de signal
    extern volatile sig_atomic_t g_sigintReceived; 
    extern volatile sig_atomic_t g_sigpipeReceived; 
    extern volatile sig_atomic_t g_displayPendingMessages; 

    // Fonctions utilitaires
    bool validatePseudo(const std::string& pseudo); 
    ProgramOptions parseArgs(int argc, char* argv[]); // Analyser les arguments
    void displayWelcome(const ProgramOptions& opts); 
    void createPipes(const std::string& sendPipe, const std::string& receivePipe); 
    void cleanupPipes(const std::string& sendPipe, const std::string& receivePipe);
    bool isPipeValid(const std::string& pipePath); 
    void notifyMessageReceived(); 
}
