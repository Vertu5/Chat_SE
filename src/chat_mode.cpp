#include "chat_mode.hpp"
#include <iostream>

namespace Chat {

void ChatMode::displayMessage(const Message& msg, bool isBot) {
    if (isBot) {
        // In bot mode: no formatting (no underline)
        std::cout << "[" << msg.from << "] " << msg.content << std::endl;
    } else {
        // Normal mode: with underline
        std::cout << "\x1B[4m[" << msg.from << "]\x1B[0m " << msg.content << std::endl;
    }
}

}

