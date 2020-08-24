#pragma once

#include <sstream>
#include <ilc/misc/helpers.h>
#include <ionir/passes/pass.h>

namespace ilc {
    class IonIrDirectiveProcessorPass : public ionir::Pass {
    private:
        OptPtr<std::stringstream> includeOutputStream;

    public:
        explicit IonIrDirectiveProcessorPass(
            OptPtr<std::stringstream> includeOutputStream = std::nullopt
        );

        void visitDirective(ionir::Directive node) override;
    };
}
