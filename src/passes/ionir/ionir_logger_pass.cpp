#include <iostream>
#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ionir/const/const.h>

namespace ilc {
    IonIrLoggerPass::IonIrLoggerPass() :
        // TODO: Temporarily passing nullptr.
        ionir::Pass(ionir::PassContext(nullptr)) {
        //
    }

    void IonIrLoggerPass::visit(ionshared::Ptr<ionir::Construct> node) {
        ionir::ConstructKind constructKind = node->getConstructKind();
        std::optional<std::string> constructName = ionir::Const::getConstructKindName(constructKind);
        std::string defaultName = "Unknown (" + std::to_string((int)constructKind) + ")";
        std::string addressString = " [" + ionshared::util::getPointerAddressString(node.get()) + "]";
        std::cout << "Visiting node: " << constructName.value_or(defaultName) << addressString << std::endl;

        ionir::Pass::visit(node);
    }
}
