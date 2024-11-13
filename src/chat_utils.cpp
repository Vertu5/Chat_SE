#include "chat.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

namespace Chat {

void ProgramOptions::print() const {
    std::cout << "\nConfiguration du chat :" << std::endl;
    std::cout << "- Utilisateur : " << user << std::endl;
    std::cout << "- Destinataire : " << dest << std::endl;
    std::string mode = "normal";
    if (isBot && isManual) mode = "bot + manuel";
    else if (isBot) mode = "bot";
    else if (isManual) mode = "manuel";
    std::cout << "- Mode : " << mode << std::endl;
}

bool validatePseudo(const std::string& pseudo) {
    // Check length (max 30 characters)
    if (pseudo.length() > MAX_PSEUDO_LENGTH) {
        std::cerr << "Erreur : pseudo trop long (max " << MAX_PSEUDO_LENGTH << " caractères)" << std::endl;
        return false;
    }

    // Check for special pseudos
    if (pseudo == "." || pseudo == "..") {
        std::cerr << "Erreur : pseudo invalide" << std::endl;
        return false;
    }

    // Check for invalid characters
    const std::string invalidChars = "/-[]";
    if (pseudo.find_first_of(invalidChars) != std::string::npos) {
        std::cerr << "Erreur : caractères invalides dans le pseudo" << std::endl;
        return false;
    }

    return true;
}

ProgramOptions parseArgs(int argc, char* argv[]) {
    // Check minimal number of arguments
    if (argc < 3) {
        std::cerr << "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]" << std::endl;
        exit(MISSING_ARGS); // Code 1
    }

    ProgramOptions opts;
    opts.user = argv[1];
    opts.dest = argv[2];

    // Check pseudonyms length
    if (opts.user.length() > MAX_PSEUDO_LENGTH || opts.dest.length() > MAX_PSEUDO_LENGTH) {
        std::cerr << "Erreur : pseudo trop long (max " << MAX_PSEUDO_LENGTH << " caractères)" << std::endl;
        exit(PSEUDO_TOO_LONG); // Code 2
    }

    // Check for invalid characters and invalid pseudos
    if (!validatePseudo(opts.user) || !validatePseudo(opts.dest)) {
        exit(INVALID_CHARS); // Code 3
    }

    // Process options --bot and --manuel
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--bot") {
            opts.isBot = true;  // Correction ici
        }
        else if (arg == "--manuel") {
            opts.isManual = true;  // Correction ici
        }
        else {
            // Unknown parameter encountered
            std::cerr << "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]" << std::endl;
            exit(MISSING_ARGS); // Code 1
        }
    }

    return opts;
}

void displayWelcome(const ProgramOptions& opts) {
    std::cout << "\n=== Chat démarré ===" << std::endl;
    opts.print();
    std::cout << "Pour quitter, tapez 'exit'" << std::endl;
    std::cout << "===================" << std::endl << std::endl;
}

void createPipes(const std::string& sendPipe, const std::string& receivePipe) {
    umask(0);
    
    if (mkfifo(sendPipe.c_str(), 0666) == -1 && errno != EEXIST) {
        std::cerr << "Erreur création pipe " << sendPipe << std::endl;
        exit(PIPE_ERROR);
    }
    
    if (mkfifo(receivePipe.c_str(), 0666) == -1 && errno != EEXIST) {
        std::cerr << "Erreur création pipe " << receivePipe << std::endl;
        unlink(sendPipe.c_str());
        exit(PIPE_ERROR);
    }
}

void cleanupPipes(const std::string& sendPipe, const std::string& receivePipe) {
    unlink(sendPipe.c_str());
    unlink(receivePipe.c_str());
}

bool isPipeValid(const std::string& pipePath) {
    struct stat statbuf;
    return (stat(pipePath.c_str(), &statbuf) == 0 && S_ISFIFO(statbuf.st_mode));
}

void notifyMessageReceived() {
    std::cout << '\a' << std::flush;
}

}

