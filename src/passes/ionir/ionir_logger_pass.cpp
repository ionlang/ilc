#include <iostream>
#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ilc/misc/log.h>
#include <ionir/const/const.h>

namespace ilc {
    IonIrLoggerPass::IonIrLoggerPass(
        ionshared::Ptr<ionshared::PassContext> context
    ) :
        ionir::Pass(std::move(context)) {
        //
    }

    void IonIrLoggerPass::visit(ionshared::Ptr<ionir::Construct> node) {
        ionir::ConstructKind constructKind = node->constructKind;
        std::optional<std::string> constructName = ionir::Const::getConstructKindName(constructKind);
        std::string defaultName = "Unknown (" + std::to_string((int)constructKind) + ")";
        std::string addressString = " [" + ionshared::util::getPointerAddressString(node.get()) + "]";

        std::cout << "Visiting: "
            << constructName.value_or(defaultName)
            << addressString
            << std::endl;

        ionir::Pass::visit(node);
    }
}
