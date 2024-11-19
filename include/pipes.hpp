/**
 * @file pipes.hpp
 * @brief Utilitaires pour la gestion des pipes
 */
#pragma once
#include "chat.hpp"
#include <fcntl.h>

namespace Chat {
namespace Pipes {

struct PipeOpenResult {
    int fd;
    bool waited;
};

// Ouverture du pipe d'écriture avec gestion de l'attente
PipeOpenResult openWritePipe(const std::string& pipePath, const ProgramOptions& opts);

// Configuration des flags pour le pipe
bool setupPipeFlags(int fd, bool nonBlocking);

// Initialisation du pipe de lecture
int initializeReadPipe(const std::string& pipePath, bool nonBlocking);

// Vérifications des erreurs de lecture/écriture
bool isNormalError(int bytesProcessed, int err);

} // namespace Pipes
} // namespace Chat

// Le namespace Chat contient les utilitaires pour la gestion des pipes
// le namespace pipes n'est pas nécessaire comme structure de namespace
// mais il est utilisé pour regrouper les fonctions liées aux pipes tout simplement 


