#include "chat.hpp"
#include "normal_mode.hpp"
#include "bot_mode.hpp"
#include "manual_mode.hpp"
#include "signal_handler.hpp"
#include <memory>
#include <sys/wait.h>

namespace Chat {
    // Initialization of global variables
    std::string g_sendPipePath;
    std::string g_receivePipePath;
    bool g_pipesOpened = false;
    volatile sig_atomic_t g_running = 1;
    volatile sig_atomic_t g_shutdown = 0;
    SignalStage g_signalStage = INIT;
    ProgramOptions g_opts;
    SharedMemory* g_sharedMemory = nullptr;

    // Signal flags
    volatile sig_atomic_t g_sigintReceived = 0;
    volatile sig_atomic_t g_sigpipeReceived = 0;
    volatile sig_atomic_t g_displayPendingMessages = 0;
}

int main(int argc, char* argv[]) {
    using namespace Chat;
    
    try {
        // Parsing arguments
        g_opts = parseArgs(argc, argv);

        // Configuration des pipe paths
        g_sendPipePath = PIPE_PATH + g_opts.user + "-" + g_opts.dest + PIPE_EXT;
        g_receivePipePath = PIPE_PATH + g_opts.dest + "-" + g_opts.user + PIPE_EXT;

        // Création des pipes
        createPipes(g_sendPipePath, g_receivePipePath);
        g_pipesOpened = true;
        g_signalStage = RUNNING;

        // Création du mode approprié
        std::unique_ptr<ChatMode> mode;
        if (g_opts.isManual) {  // On vérifie d'abord le mode manuel
            mode.reset(new ManualMode(g_opts, g_sendPipePath, g_receivePipePath));
        } else if (g_opts.isBot) {  // Ensuite le mode bot si pas de mode manuel
            mode.reset(new BotMode(g_opts, g_sendPipePath, g_receivePipePath));
        } else {
            mode.reset(new NormalMode(g_opts, g_sendPipePath, g_receivePipePath));
        }

        // Fork du processus
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Erreur fork" << std::endl;
            cleanupPipes(g_sendPipePath, g_receivePipePath);
            return SYSTEM_ERROR;
        }

        if (pid == 0) {
            // Processus enfant
            // L'enfant ignore SIGINT
            signal(SIGINT, SIG_IGN);
            mode->runChildProcess();
            exit(SUCCESS);
        } else {
            // Processus parent
            // Configuration des gestionnaires de signaux
            setupSignalHandlers();

            // Affichage du message de bienvenue
            displayWelcome(g_opts);

            mode->runParentProcess();

            // Nettoyage
            g_running = 0;
            kill(pid, SIGTERM);
            int status;
            waitpid(pid, &status, 0);
            
            cleanupPipes(g_sendPipePath, g_receivePipePath);
            
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return SUCCESS;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Erreur critique : " << e.what() << std::endl;
        if (g_pipesOpened) {
            cleanupPipes(g_sendPipePath, g_receivePipePath);
        }
        return SYSTEM_ERROR;
    }
}

