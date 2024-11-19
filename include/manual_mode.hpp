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
        std::unique_ptr<SharedMemory> sharedMem;
        bool displayPendingMessages;

    public:
        ManualMode(const ProgramOptions& opts,
                   const std::string& sendPipe,
                   const std::string& receivePipe);
        ~ManualMode() = default;  // Le unique_ptr s'occupera du nettoyage

        void runParentProcess() override;
        void runChildProcess() override;
    };
}