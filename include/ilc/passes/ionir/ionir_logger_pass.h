#pragma once

#include <ionir/passes/pass.h>

namespace ilc {
    struct IonIrLoggerPass : ionir::Pass {
        IONSHARED_PASS_ID;

        explicit IonIrLoggerPass(
            std::shared_ptr<ionshared::PassContext> context
        );

        void visit(std::shared_ptr<ionir::Construct> node) override;
    };
}
