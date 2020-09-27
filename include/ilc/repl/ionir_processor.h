#pragma once

#include <string>
#include <ionshared/misc/helpers.h>
#include <ionir/construct/module.h>
#include <ionlang/lexical/token.h>
#include "repl_processor.h"

namespace ilc {
    class IonIrProcessor : public ReplProcessor {
    private:
        ionshared::Ptr<ionir::Module> module;

        ionlang::TokenStream tokenStream;

    public:
        IonIrProcessor(
            Options options,
            ionshared::Ptr<ionir::Module> module,
            ionlang::TokenStream tokenStream
        );

        void run() override;
    };
}
