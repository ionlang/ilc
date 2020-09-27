#include <ilc/repl/repl_processor.h>

namespace ilc {
    void ReplProcessor::tryThrow(std::exception exception) {
        if (this->options.replThrow) {
            throw exception;
        }
    }

    ReplProcessor::ReplProcessor(Options options, std::string input) :
        options(options),
        input(input) {
        //
    }
}
