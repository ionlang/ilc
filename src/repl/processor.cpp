#include <ionshared/diagnostics/diagnostic.h>
#include <ionshared/llvm/llvm_module.h>
#include <ionir/passes/codegen/llvm_codegen_pass.h>
#include <ionir/passes/type_system/type_check_pass.h>
#include <ionir/passes/type_system/borrow_check_pass.h>
#include <ionir/passes/semantic/entry_point_check_pass.h>
#include <ionlang/passes/lowering/ionir_lowering_pass.h>
#include <ionlang/passes/semantic/macro_expansion_pass.h>
#include <ionlang/passes/semantic/name_resolution_pass.h>
#include <ionlang/lexical/lexer.h>
#include <ionlang/syntax/parser.h>
#include <ilc/passes/ionlang/ionlang_logger_pass.h>
#include <ilc/diagnostics/diagnostic_printer.h>
#include <ilc/repl/processor.h>

namespace ilc {
    std::vector<ionlang::Token> IonLangProcessor::lex() {
        ionlang::Lexer lexer = ionlang::Lexer(this->input);
        std::vector<ionlang::Token> tokens = lexer.scan();

        std::cout << "--- Lexer: " << tokens.size() << " token(s) ---" << std::endl;

        for (auto token : tokens) {
            std::cout << token << std::endl;
        }

        return tokens;
    }

    ionshared::OptPtr<ionlang::Module> IonLangProcessor::parse(
        std::vector<ionlang::Token> tokens,
        ionshared::Ptr<DiagnosticVector> diagnostics
    ) {
        ionlang::TokenStream tokenStream = ionlang::TokenStream(tokens);

        ionlang::Parser parser = ionlang::Parser(
            tokenStream,
            std::make_shared<ionshared::DiagnosticBuilder>(diagnostics)
        );

        this->tokenStream = tokenStream;

        try {
            ionlang::AstPtrResult<ionlang::Module> moduleResult = parser.parseModule();

            // TODO: Improve if block?
            if (ionlang::util::hasValue(moduleResult)) {
                // TODO: What if multiple top-level, in-line constructs are parsed? (Additional note below).
                std::cout << "--- Parser ---" << std::endl;

                return ionlang::util::getResultValue(moduleResult);
            }

            std::cout << "Parser: [Exception] Could not parse module" << std::endl;

            DiagnosticPrinter diagnosticPrinter = DiagnosticPrinter(DiagnosticPrinterOpts{
                this->input,
                tokenStream
            });

            DiagnosticPrinterResult printResult =
                diagnosticPrinter.createDiagnosticStackTrace(diagnostics);

            // TODO: Check for null ->make().
            if (printResult.first.has_value()) {
                std::cout << *printResult.first;
                std::cout.flush();
            }
            else {
                std::cout << "Could not create stack-trace" << std::endl;
            }
        }
        catch (std::exception &exception) {
            std::cout << "Parser: [Exception] " << exception.what() << std::endl;
            this->tryThrow(exception);
        }

        return std::nullopt;
    }

    void IonLangProcessor::codegen(
        ionshared::Ptr<ionlang::Module> module,
        ionshared::Ptr<DiagnosticVector> diagnostics
    ) {
        try {
            // TODO: Creating mock AST?
            ionlang::Ast ionLangAst = {
                module
            };

            /**
             * Create a pass manager instance & run applicable passes
             * over the resulting AST.
             */
            ionlang::PassManager ionLangPassManager = ionlang::PassManager();

            ionshared::Ptr<ionshared::PassContext> passContext =
                std::make_shared<ionshared::PassContext>(diagnostics);

            // Register all passes to be used by the pass manager.
            // TODO: Create and implement IonLangLogger pass.
            if (this->options.passes.contains(PassKind::IonLangLogger)) {
                ionLangPassManager.registerPass(std::make_shared<IonLangLoggerPass>(passContext));
            }

            if (this->options.passes.contains(PassKind::MacroExpansion)) {
                ionLangPassManager.registerPass(std::make_shared<ionlang::MacroExpansionPass>(passContext));
            }

            if (this->options.passes.contains(PassKind::NameResolution)) {
                ionLangPassManager.registerPass(std::make_shared<ionlang::NameResolutionPass>(passContext));
            }

            // Execute the pass manager against the parser's resulting AST.
            ionLangPassManager.run(ionLangAst);

            // TODO: CRITICAL: Should be used with the PassManager instance, as a normal pass instead of manually invoking the visit functions.
            ionlang::IonIrLoweringPass ionIrLoweringPass = ionlang::IonIrLoweringPass(passContext);

            // TODO: What if multiple top-level constructs are defined in-line? Use ionir::Driver (finish it first) and use its resulting Ast. (Additional note above).
            // Visit the parsed module construct.
            ionIrLoweringPass.visitModule(module);

            ionshared::OptPtr<ionir::Module> ionIrModuleBuffer = ionIrLoweringPass.getModuleBuffer();

            if (!ionshared::util::hasValue(ionIrModuleBuffer)) {
                throw std::runtime_error("Module is nullptr");
            }

            ionir::Ast ionIrAst = {
                *ionIrModuleBuffer
            };

            ionir::PassManager ionirPassManager = ionir::PassManager();

            // Register passes.
            if (this->options.passes.contains(PassKind::EntryPointCheck)) {
                ionirPassManager.registerPass(std::make_shared<ionir::EntryPointCheckPass>(passContext));
            }

            if (this->options.passes.contains(PassKind::TypeChecking)) {
                ionirPassManager.registerPass(std::make_shared<ionir::TypeCheckPass>(passContext));
            }

            if (this->options.passes.contains(PassKind::BorrowCheck)) {
                ionirPassManager.registerPass(std::make_shared<ionir::BorrowCheckPass>(passContext));
            }

            // Run the pass manager on the IonIR AST.
            ionirPassManager.run(ionIrAst);

            DiagnosticPrinter diagnosticPrinter = DiagnosticPrinter(DiagnosticPrinterOpts{
                this->input,
                *this->tokenStream
            });

            DiagnosticPrinterResult printResult =
                diagnosticPrinter.createDiagnosticStackTrace(diagnostics);

            // TODO: Blocking multi-modules?
            if (printResult.second > 0) {
                std::cout << " --- LLVM code-generation: Error(s) encountered ---" << std::endl;

                return;
            }

            // TODO: Where should optimization passes occur? Before or after type-checking?
//            ionirPassManager.registerPass(std::make_shared<ionir::DeadCodeEliminationPass>());

            // Now, make the ionir::LlvmCodegenPass.
            ionir::LlvmCodegenPass ionIrLlvmCodegenPass = ionir::LlvmCodegenPass(passContext);

            // Visit the resulting IonIR module buffer from the IonLang codegen pass.
            ionIrLlvmCodegenPass.visitModule(*ionIrModuleBuffer);

            std::map<std::string, llvm::Module *> modules = ionIrLlvmCodegenPass.getModules()->unwrap();

            // Display the resulting code of all the modules.
            for (const auto &[key, value] : modules) {
                std::cout << "--- LLVM code-generation: " << key << " ---" << std::endl;
                ionshared::LlvmModule(value).print();
            }

            if (modules.empty()) {
                std::cout << "--- LLVM code-generation contained no modules ---" << std::endl;
            }
        }
        catch (std::exception &exception) {
            std::cout << "LLVM code-generation: [Exception] " << exception.what() << std::endl;
            this->tryThrow(exception);
        }
    }

    IonLangProcessor::IonLangProcessor(Options options, std::string input) :
        ReplProcessor(options, input),
        tokenStream(std::nullopt) {
        //
    }

    void IonLangProcessor::run() {
        std::vector<ionlang::Token> tokens = this->lex();

        ionshared::Ptr<DiagnosticVector> diagnostics =
            std::make_shared<DiagnosticVector>();

        ionshared::OptPtr<ionlang::Module> module = this->parse(tokens, diagnostics);

        if (ionshared::util::hasValue(module)) {
            this->codegen(*module, diagnostics);
        }
    }
}
