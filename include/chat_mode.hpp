#pragma once
#include "chat.hpp"

namespace Chat {
    class ChatMode {
    protected:
        const ProgramOptions& opts;
        const std::string sendPipe;
        const std::string receivePipe;

    public:
        ChatMode(const ProgramOptions& options, 
                const std::string& send, 
                const std::string& receive) 
            : opts(options), sendPipe(send), receivePipe(receive) {}
        
        virtual ~ChatMode() = default;
        virtual void runParentProcess() = 0;
        virtual void runChildProcess() = 0;
        
        static void displayMessage(const Message& msg, bool isBot);
    };
}

