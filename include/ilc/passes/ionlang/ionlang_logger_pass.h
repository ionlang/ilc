#pragma once

#include <ionlang/passes/pass.h>

namespace ilc {
    class IonLangLoggerPass : public ionlang::Pass {
    public:
        void visit(ionshared::Ptr<ionlang::Construct> node) override;
    };
}
