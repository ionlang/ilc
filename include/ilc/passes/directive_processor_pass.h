#pragma once

#include <ionir/passes/pass.h>

namespace ilc {
    class DirectiveProcessorPass : public ionir::Pass {
    public:
        void visitDirective(ionir::Directive node) override;
    };
}
