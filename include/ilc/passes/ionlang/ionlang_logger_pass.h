#pragma once

#include <ionlang/passes/pass.h>

namespace ilc {
    struct IonLangLoggerPass : ionlang::Pass {
        IONSHARED_PASS_ID;

        explicit IonLangLoggerPass(
            ionshared::Ptr<ionshared::PassContext> context
        );

        void visit(ionshared::Ptr<ionlang::Construct> node) override;
    };
}
