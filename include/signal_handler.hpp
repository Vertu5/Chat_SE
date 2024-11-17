#pragma once
#include <signal.h>
#include "chat.hpp"

namespace Chat {
    void setupSignalHandlers();
    void signalHandler(int signal);
}
