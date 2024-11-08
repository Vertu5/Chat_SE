#pragma once
#include <signal.h>

namespace Chat {
    void setupSignalHandlers();
    void signalHandler(int signal);
}

