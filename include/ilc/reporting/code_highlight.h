#pragma once

#include <string>
#include <ionlang/lexical/token.h>

namespace ilc {
    class CodeHighlight {
    public:
        static std::string coat(ionlang::Token token);
    };
}
