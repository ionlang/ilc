#include <ilc/passes/directive_processor_pass.h>

namespace ilc {
    void DirectiveProcessorPass::visitDirective(ionir::Directive node) {
        std::string directiveName = node.first;

        // TODO: Hard-coded string(s).
        if (directiveName == "import") {
            // TODO: Implement.
        }
        else if (directiveName == "define") {
            // TODO: Implement.
        }
    }
}
