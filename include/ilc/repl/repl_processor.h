#pragma once

#include <string>
#include <ilc/cli/options.h>

namespace ilc {
    class ReplProcessor {
    private:
        Options options;

        std::string input;

    protected:
        void tryThrow(std::exception exception);

    public:
        explicit ReplProcessor(Options options, std::string input);

        virtual void run() = 0;

        [[nodiscard]] std::string getInput() const noexcept;

        void setInput(std::string input) noexcept;

        [[nodiscard]] Options getOptions() const noexcept;

        void setOptions(Options options) noexcept;
    };
}
