#pragma once

#include <ionir/passes/pass.h>

namespace ilc {
    class IonIrLoggerPass : public ionir::Pass {
    public:
        IonIrLoggerPass();

        void visit(ionshared::Ptr<ionir::Construct> node) override;
    };
}
