#pragma once

#include <ionir/passes/pass.h>

namespace ilc {
    class IonIrLoggerPass : public ionir::Pass {
    public:
        IONSHARED_PASS_ID;

        explicit IonIrLoggerPass(
            ionshared::Ptr<ionshared::PassContext> context
        );

        void visit(ionshared::Ptr<ionir::Construct> node) override;
    };
}
