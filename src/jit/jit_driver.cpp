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
#include <ilc/misc/log.h>
#include <ilc/jit/jit_driver.h>

namespace ilc {
    std::vector<ionlang::Token> JitDriver::lex() {
        ionlang::Lexer lexer = ionlang::Lexer(this->input);
        std::vector<ionlang::Token> tokens = lexer.scan();

        std::cout
            << ConsoleColor::coat(
                "--- Lexer: " + std::to_string(tokens.size()) + " token(s) ---",
                ColorKind::ForegroundGreen
            )

            << std::endl;

        for (auto token : tokens) {
            std::cout << token << std::endl;
        }

        return tokens;
    }

    ionshared::OptPtr<ionlang::Module> JitDriver::parse(
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
                std::cout << ConsoleColor::coat("--- Parser ---", ColorKind::ForegroundGreen)
                    << std::endl;

                return ionlang::util::getResultValue(moduleResult);
            }

            log::error("Parser: Could not parse module");

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
                log::error("Could not create stack-trace");
            }
        }
        catch (std::exception &exception) {
            log::error("Parser: " + std::string(exception.what()));
            this->tryThrow(exception);
        }

        return std::nullopt;
    }

    void JitDriver::codegen(
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
            if (cli::options.passes.contains(cli::PassKind::IonLangLogger)) {
                ionLangPassManager.registerPass(std::make_shared<IonLangLoggerPass>(passContext));
            }

            if (cli::options.passes.contains(cli::PassKind::MacroExpansion)) {
                ionLangPassManager.registerPass(std::make_shared<ionlang::MacroExpansionPass>(passContext));
            }

            if (cli::options.passes.contains(cli::PassKind::NameResolution)) {
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
            if (cli::options.passes.contains(cli::PassKind::EntryPointCheck)) {
                ionirPassManager.registerPass(std::make_shared<ionir::EntryPointCheckPass>(passContext));
            }

            if (cli::options.passes.contains(cli::PassKind::TypeChecking)) {
                ionirPassManager.registerPass(std::make_shared<ionir::TypeCheckPass>(passContext));
            }

            if (cli::options.passes.contains(cli::PassKind::BorrowCheck)) {
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
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation: " + key + " ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;

                ionshared::LlvmModule(value).printIr();
            }

            if (modules.empty()) {
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation contained no modules ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;
            }
        }
        catch (std::exception &exception) {
            log::error("LLVM code-generation: " + std::string(exception.what()));
            this->tryThrow(exception);
        }
    }

    void JitDriver::tryThrow(std::exception exception) {
        if (cli::options.jitThrow) {
            throw exception;
        }
    }

    void JitDriver::run(std::string input) {
        this->input = input;

        std::vector<ionlang::Token> tokens = this->lex();

        ionshared::Ptr<DiagnosticVector> diagnostics =
            std::make_shared<DiagnosticVector>();

        ionshared::OptPtr<ionlang::Module> module = this->parse(tokens, diagnostics);

        if (ionshared::util::hasValue(module)) {
            this->codegen(*module, diagnostics);
        }
    }
}
