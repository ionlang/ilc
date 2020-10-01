#include <cstdlib>
#include <ilc/cli/log.h>
#include <ilc/jit/jit.h>

namespace ilc::jit {
    void registerCommonActions() {
        jit::actions.set("quit", []() {
            // TODO: std::exit is not a safe method to exit the program.
            std::exit(EXIT_SUCCESS);
        });

        jit::actions.set("clear", []() {
            #if defined(OS_WINDOWS)
                system("cls");
            #elif defined(OS_LINUX) || defined(OS_MAC)
                system("clear");
            #else
                log::error("That action is platform-specific, and this platform is unknown or unsupported");
            #endif
        });
    }
}
