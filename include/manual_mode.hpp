#pragma once
#include "chat_mode.hpp"
#include "shared_memory.hpp"

namespace Chat {
    class ManualMode : public ChatMode {
    private:
        SharedMemory* sharedMem;
        bool displayPendingMessages;

    public:
        ManualMode(const ProgramOptions& opts,
                  const std::string& sendPipe,
                  const std::string& receivePipe);
        ~ManualMode();

        void runParentProcess() override;
        void runChildProcess() override;
    };
}

