#pragma once

#include <ionir/passes/pass.h>

namespace ilc {
    struct IonIrLoggerPass : ionir::Pass {
        IONSHARED_PASS_ID;

        explicit IonIrLoggerPass(
            ionshared::Ptr<ionshared::PassContext> context
        );

        void visit(ionshared::Ptr<ionir::Construct> node) override;
    };
}
