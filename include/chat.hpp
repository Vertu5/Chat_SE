#pragma once
#include <string>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <unistd.h> 
#include "shared_memory.hpp"

namespace Chat {
    // Constants
    const int MAX_PSEUDO_LENGTH = 30;
    const std::string PIPE_PATH = "/tmp/";
    const std::string PIPE_EXT = ".chat";

    // Return codes
    enum ReturnCodes {
        SUCCESS = 0,
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

    enum SignalStage {
        INIT,
        RUNNING
    };

    // Structure for messages
    struct Message {
        char from[MAX_PSEUDO_LENGTH + 1];
        char to[MAX_PSEUDO_LENGTH + 1];
        char content[1024];

        Message() {
            memset(from, 0, sizeof(from));
            memset(to, 0, sizeof(to));
            memset(content, 0, sizeof(content));
        }
    };

    // Structure for program options
    struct ProgramOptions {
        std::string user;
        std::string dest;
        bool isBot = false;
        bool isManual = false;
        
        void print() const;
    };

    // Global variables
    extern std::string g_sendPipePath;
    extern std::string g_receivePipePath;
    extern bool g_pipesOpened;
    extern volatile sig_atomic_t g_running;
    extern volatile sig_atomic_t g_shutdown;
    extern SignalStage g_signalStage;
    extern ProgramOptions g_opts;
    extern SharedMemory* g_sharedMemory;

    // Signal flags
    extern volatile sig_atomic_t g_sigintReceived;
    extern volatile sig_atomic_t g_sigpipeReceived;
    extern volatile sig_atomic_t g_displayPendingMessages;


    // Utility functions
    bool validatePseudo(const std::string& pseudo);
    ProgramOptions parseArgs(int argc, char* argv[]);
    void displayWelcome(const ProgramOptions& opts);
    void createPipes(const std::string& sendPipe, const std::string& receivePipe);
    void cleanupPipes(const std::string& sendPipe, const std::string& receivePipe);
    bool isPipeValid(const std::string& pipePath);
    void notifyMessageReceived();
}
