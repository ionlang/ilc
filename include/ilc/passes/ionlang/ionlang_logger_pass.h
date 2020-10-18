#pragma once

#include <ionlang/passes/pass.h>

namespace ilc {
    struct IonLangLoggerPass : ionlang::Pass {
        IONSHARED_PASS_ID;

        explicit IonLangLoggerPass(
            std::shared_ptr<ionshared::PassContext> context
        );

        void visit(std::shared_ptr<ionlang::Construct> node) override;
    };
}
