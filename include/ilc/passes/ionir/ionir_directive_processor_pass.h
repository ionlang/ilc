#pragma once

#include <sstream>
#include <ionir/passes/pass.h>

namespace ilc {
    class IonIrDirectiveProcessorPass : public ionir::Pass {
    private:
        ionshared::OptPtr<std::stringstream> includeOutputStream;

    public:
        IONSHARED_PASS_ID;

        explicit IonIrDirectiveProcessorPass(
            ionshared::Ptr<ionshared::PassContext> context,
            ionshared::OptPtr<std::stringstream> includeOutputStream = std::nullopt
        );

        void visitDirective(ionir::Directive node) override;
    };
}
