#pragma once
#include "normal_mode.hpp"

namespace Chat {
    class BotMode : public NormalMode {
    public:
        BotMode(const ProgramOptions& opts,
                const std::string& sendPipe,
                const std::string& receivePipe);
    };
}

