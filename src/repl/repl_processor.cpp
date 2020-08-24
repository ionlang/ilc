#include <ilc/repl/repl_processor.h>

namespace ilc {
    void ReplProcessor::tryThrow(std::exception exception) {
        if (this->options.replThrow) {
            throw exception;
        }
    }

    ReplProcessor::ReplProcessor(Options options, std::string input)
        : options(options), input(input) {
        //
    }

    std::string ReplProcessor::getInput() const noexcept {
        return this->input;
    }

    void ReplProcessor::setInput(std::string input) noexcept {
        this->input = std::move(input);
    }

    Options ReplProcessor::getOptions() const noexcept {
        return this->options;
    }

    void ReplProcessor::setOptions(Options options) noexcept {
        this->options = std::move(options);
    }
}
