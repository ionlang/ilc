#pragma once

#include <string>
#include <ilc/cli/options.h>

namespace ilc {
    class ReplProcessor {
    protected:
        void tryThrow(std::exception exception);

    public:
        std::string input;

        Options options;

        ReplProcessor(Options options, std::string input);

        virtual void run() = 0;
    };
}
