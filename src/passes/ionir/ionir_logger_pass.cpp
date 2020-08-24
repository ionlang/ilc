#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ionir/const/const.h>

namespace ilc {
    void IonIrLoggerPass::visit(ionshared::Ptr<ionir::Construct> node) {
        ionir::ConstructKind constructKind = node->getConstructKind();
        std::optional<std::string> constructName = ionir::Const::getConstructKindName(constructKind);
        std::string defaultName = "Unknown (" + std::to_string((int)constructKind) + ")";
        std::cout << "Visiting node: " << constructName.value_or(defaultName) << std::endl;

        ionir::Pass::visit(node);
    }
}
