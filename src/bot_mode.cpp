#include "bot_mode.hpp"

namespace Chat {

BotMode::BotMode(const ProgramOptions& opts,
                 const std::string& sendPipe,
                 const std::string& receivePipe)
    : NormalMode(opts, sendPipe, receivePipe) {
    // Bot mode inherits from NormalMode
    // The only difference is that displayMessage is called with isBot=true
    // and sent messages are not displayed (handled in NormalMode)
}

}

