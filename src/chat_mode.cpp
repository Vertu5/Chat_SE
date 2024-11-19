#include "chat_mode.hpp"
#include <iostream>

namespace Chat {

void ChatMode::displayMessage(const Message& msg, bool isBot) {
    if (g_opts.isJoli) {
        bool shouldIndent = false;
        
        // Vérifie si c'est un message reçu (on est le destinataire)
        if (msg.from != g_opts.user && msg.to == g_opts.user) {
            shouldIndent = true;
        }

        if (shouldIndent) {
            std::cout << std::string(40, ' ');
        }
    }
    
    if (isBot) {
        std::cout << "[" << msg.from << "] " << msg.content << std::endl;
    } else {
        std::cout << "\x1B[4m[" << msg.from << "]\x1B[0m " << msg.content << std::endl;
    }
}

}

