#pragma once

#include <ionlang/passes/pass.h>

namespace ilc {
    class IonLangPrinterPass : public ionlang::Pass {
    private:
        uint32_t depth;

        std::string makeSpaces(uint32_t length) const noexcept;

    public:
        IONSHARED_PASS_ID;

        const uint32_t maxDepth;

        explicit IonLangPrinterPass(
            ionshared::Ptr<ionshared::PassContext> context,
            uint32_t maxDepth = 100
        );

        void visit(ionshared::Ptr<ionlang::Construct> construct) override;
    };
}
