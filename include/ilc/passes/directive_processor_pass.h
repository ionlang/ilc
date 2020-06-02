#pragma once

#include <sstream>
#include <ilc/misc/helpers.h>
#include <ionir/passes/pass.h>

namespace ilc {
    class DirectiveProcessorPass : public ionir::Pass {
    private:
        OptPtr<std::stringstream> includeOutputStream;

    public:
        explicit DirectiveProcessorPass(OptPtr<std::stringstream> includeOutputStream = std::nullopt);

        void visitDirective(ionir::Directive node) override;
    };
}
