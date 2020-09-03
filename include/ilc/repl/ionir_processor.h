#pragma once

#include <string>
#include <ionshared/misc/helpers.h>
#include <ionir/lexical/lexer.h>
#include <ionir/construct/module.h>
#include "repl_processor.h"

namespace ilc {
    class IonIrProcessor : public ReplProcessor {
    private:
        ionshared::Ptr<ionir::Module> module;

    public:
        IonIrProcessor(Options options, ionshared::Ptr<ionir::Module> module);

        void run() override;
    };
}
