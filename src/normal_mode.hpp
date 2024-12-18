/**
 * @file normal_mode.hpp 
 * @brief Définition du mode normal du chat (utilisateur)
 */

#pragma once
#include "chat_mode.hpp"

namespace Chat {
    class NormalMode : public ChatMode {
    public:
        NormalMode(const ProgramOptions& opts, 
                  const std::string& sendPipe, 
                  const std::string& receivePipe);
        void runParentProcess() override;
        void runChildProcess() override;
    };
}

