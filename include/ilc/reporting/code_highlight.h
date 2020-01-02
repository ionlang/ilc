#pragma once

#include <string>
#include <ionir/lexical/token.h>

namespace ilc {
    class CodeHighlight {
    public:
        static std::string coat(ionir::Token token);
    };
}
