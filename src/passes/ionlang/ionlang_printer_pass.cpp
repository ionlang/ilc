#include <iostream>
#include <ilc/passes/ionlang/ionlang_printer_pass.h>

namespace ilc {
    std::string IonLangPrinterPass::makeSpaces(uint32_t length) const noexcept {
        std::stringstream result;
        uint32_t counter = 0;

        while (counter != length) {
            result << " ";
            counter++;
        }

        return result.str();
    }

    IonLangPrinterPass::IonLangPrinterPass(
        std::shared_ptr<ionshared::PassContext> context,
        uint32_t maxDepth
    ) :
        ionlang::Pass(context),
        maxDepth(maxDepth) {
        //
    }

    void IonLangPrinterPass::visit(std::shared_ptr<ionlang::Construct> construct) {
        std::cout
            << this->makeSpaces(this->depth)
            << "Node: "
            << construct->findConstructName().value_or("Unknown");

        this->depth++;
        ionlang::Pass::visit(construct);
        this->depth--;
    }
}
