#pragma once

#include <sstream>
#include <ilc/misc/helpers.h>
#include <ionir/passes/pass.h>

namespace ilc {
    class IonIrDirectiveProcessorPass : public ionir::Pass {
    private:
        OptPtr<std::stringstream> includeOutputStream;

    public:
        IONSHARED_PASS_ID;

        explicit IonIrDirectiveProcessorPass(
            ionshared::Ptr<ionshared::PassContext> context,
            OptPtr<std::stringstream> includeOutputStream = std::nullopt
        );

        void visitDirective(ionir::Directive node) override;
    };
}
