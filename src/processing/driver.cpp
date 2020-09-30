#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/CommandFlags.inc"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/CodeGen/MIRParser/MIRParser.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/AutoUpgrade.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/RemarkStreamer.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"

///////////////////////////////////////////

#include <memory>
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
#include <ilc/processing/driver.h>

namespace ilc {
    std::vector<ionlang::Token> Driver::lex() {
        ionlang::Lexer lexer = ionlang::Lexer(this->input);
        std::vector<ionlang::Token> tokens = lexer.scan();

        std::cout
            << ConsoleColor::coat(
                "--- Lexer: " + std::to_string(tokens.size()) + " token(s) ---",
                ColorKind::ForegroundGreen
            )

            << std::endl;

        uint32_t counter = 0;

        for (auto &token : tokens) {
            // TODO: Only do if specified by option 'trim'.
            if (counter == 10) {
                std::cout << ConsoleColor::coat("... trimmed ...", ColorKind::ForegroundGray)
                    << std::endl;

                break;
            }

            std::cout << token << std::endl;
            counter++;
        }

        return tokens;
    }

    ionshared::OptPtr<ionlang::Module> Driver::parse(
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

    std::optional<std::vector<llvm::Module *>> Driver::lowerToLlvmIr(
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

//            if (cli::options.passes.contains(cli::PassKind::NameResolution)) {
                ionLangPassManager.registerPass(std::make_shared<ionlang::NameResolutionPass>(passContext));
//            }

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

                return std::nullopt;
            }

            // TODO: Where should optimization passes occur? Before or after type-checking?
//            ionirPassManager.registerPass(std::make_shared<ionir::DeadCodeEliminationPass>());

            // Now, make the ionir::LlvmCodegenPass.
            ionir::LlvmCodegenPass ionIrLlvmCodegenPass = ionir::LlvmCodegenPass(passContext);

            // Visit the resulting IonIR module buffer from the IonLang codegen pass.
            ionIrLlvmCodegenPass.visitModule(*ionIrModuleBuffer);

            std::map<std::string, llvm::Module *> modules = ionIrLlvmCodegenPass.getModules()->unwrap();

            if (modules.empty()) {
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation contained no modules ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;

                return std::nullopt;
            }

            std::vector<llvm::Module *> result = std::vector<llvm::Module *>();

            // Display the resulting code of all the modules.
            for (const auto &[key, value] : modules) {
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation: " + key + " ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;

                ionshared::LlvmModule llvmModule = ionshared::LlvmModule(value);

                llvmModule.printIr();
                result.push_back(value);
            }

            return result;
        }
        catch (std::exception &exception) {
            log::error("LLVM code-generation: " + std::string(exception.what()));
            this->tryThrow(exception);
        }

        return std::nullopt;
    }

    void Driver::makeObjectCode(llvm::Triple llvmTriple, llvm::Module *llvmModule) {
        // Note: Code taken and adjusted from https://github.com/llvm-mirror/llvm/blob/master/tools/llc/llc.cpp#L491.

        std::string error;

        // Initialize the native host target.
        llvm::InitializeNativeTarget();

        const llvm::Target *llvmTarget = llvm::TargetRegistry::lookupTarget(
            llvmTriple.getTriple(),
            error
        );

        if (!llvmTarget) {
            log::error("Make object code: " + error);

            // TODO
            throw std::runtime_error("Exit error");
//            return std::nullopt;
        }

        std::string cpuString = getCPUStr();
        std::string featuresString = getFeaturesStr();

        llvm::TargetOptions llvmTargetOptions = InitTargetOptionsFromCodeGenFlags();

        // Specify whether to preserve comments on the emitted assembly code.
        llvmTargetOptions.MCOptions.PreserveAsmComments = true;

        std::unique_ptr<llvm::TargetMachine> llvmTargetMachine =
            std::unique_ptr<llvm::TargetMachine>(llvmTarget->createTargetMachine(
                llvmTriple.getTriple(),
                cpuString,
                featuresString,
                llvmTargetOptions,
                getRelocModel(),
                getCodeModel(),

                // TODO: Edit the optimization level enum.
                CodeGenOpt::Default
            ));

        if (!llvmTargetMachine) {
            log::error("Make object code: Could not allocate target machine");

            // TODO
            throw std::runtime_error("Exit error");
//            return std::nullopt;
        }

        llvm::legacy::PassManager llvmPassManager;

        llvm::TargetLibraryInfoImpl llvmTargetLibraryInfoImpl =
            llvm::TargetLibraryInfoImpl(llvm::Triple(llvmModule->getTargetTriple()));

        llvmPassManager.add(new TargetLibraryInfoWrapperPass(llvmTargetLibraryInfoImpl));

//        llvmModule->setDataLayout(Target->createDataLayout());

        // TODO: Import code to verify module.

        setFunctionAttributes(cpuString, featuresString, *llvmModule);

        llvm::raw_ostream *outputStream = &llvm::outs();
        // TODO: std;:error_code() for &outs()?

        llvm::SmallVector<char, 0> buffer;

        std::unique_ptr<raw_svector_ostream> bos =
            std::make_unique<raw_svector_ostream>(buffer);

        llvm::LLVMTargetMachine &llvmLlvmTargetMachine = static_cast<llvm::LLVMTargetMachine &>(*llvmTargetMachine);

        llvm::MachineModuleInfo *llvmMachineModuleInfoPass = new MachineModuleInfo(&llvmLlvmTargetMachine);

        // TODO: Tweak for support for assembly too.
        bool NoVerify = true;

        llvmTargetMachine->addPassesToEmitFile(
            llvmPassManager,
            *bos.get(),
            nullptr,
            llvm::TargetMachine::CodeGenFileType::CGFT_ObjectFile,
            NoVerify,
            llvmMachineModuleInfoPass
        );

        // TODO: Code in between.

        llvmPassManager.run(*llvmModule);

        // TODO: HasError.

        llvm::outs() << buffer;
    }

    void Driver::makeObjectCode2(llvm::Triple llvmTriple, llvm::Module *M) {
        std::cout << "begin make object code 2" << std::endl;

        // Initialize targets first, so that --version shows registered targets.
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmPrinters();
        InitializeAllAsmParsers();

        // Initialize codegen and IR passes used by llc so that the -print-after,
        // -print-before, and -stop-after options work.
        PassRegistry *Registry = PassRegistry::getPassRegistry();
        initializeCore(*Registry);
        initializeCodeGen(*Registry);
        initializeLoopStrengthReducePass(*Registry);
        initializeLowerIntrinsicsPass(*Registry);
        initializeEntryExitInstrumenterPass(*Registry);
        initializePostInlineEntryExitInstrumenterPass(*Registry);
        initializeUnreachableBlockElimLegacyPassPass(*Registry);
        initializeConstantHoistingLegacyPassPass(*Registry);
        initializeScalarOpts(*Registry);
        initializeVectorization(*Registry);
        initializeScalarizeMaskedMemIntrinPass(*Registry);
        initializeExpandReductionsPass(*Registry);
        initializeHardwareLoopsPass(*Registry);

        // Initialize debugging passes.
        initializeScavengerTestPass(*Registry);

        std::string arg0 = "llvmout";
        std::string InputFilename = "llvminput";
        bool NoVerify = false;
        auto FT = llvm::TargetMachine::CodeGenFileType::CGFT_ObjectFile;

        // Load the module to be compiled...
        SMDiagnostic Err;
        std::unique_ptr<MIRParser> MIR;
        Triple TheTriple = Triple(sys::getDefaultTargetTriple());

        // Get the target specific parser.
        std::string Error;

        const Target *TheTarget = TargetRegistry::lookupTarget(
            TheTriple.getTriple(),
            Error
        );

        if (!TheTarget) {
            throw std::runtime_error("Could not create target: " + Error);
        }

        std::string CPUStr = getCPUStr();
        std::string FeaturesStr = getFeaturesStr();
        CodeGenOpt::Level OLvl = CodeGenOpt::Default;

        TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

        std::unique_ptr<TargetMachine> Target(TheTarget->createTargetMachine(
            TheTriple.getTriple(), CPUStr, FeaturesStr, Options, getRelocModel(),
            getCodeModel(), OLvl));

        assert(Target && "Could not allocate target machine!");

        assert(M && "Should have exited if we didn't have a module!");

        if (FloatABIForCalls != FloatABI::Default)
            Options.FloatABIType = FloatABIForCalls;

        // Figure out where we are going to send the output.
        raw_ostream *outputStream = &llvm::outs();

        // Build up all of the passes that we want to do to the module.
        legacy::PassManager PM;

        // Add an appropriate TargetLibraryInfo pass for the module's triple.
        TargetLibraryInfoImpl TLII(Triple(M->getTargetTriple()));

        PM.add(new TargetLibraryInfoWrapperPass(TLII));

        auto dl = Target->createDataLayout();

        // Add the target data from the target machine, if it exists, or the module.
        M->setDataLayout(dl);

        // This needs to be done after setting datalayout since it calls verifier
        // to check debug info whereas verifier relies on correct datalayout.
        UpgradeDebugInfo(*M);

        // Verify module immediately to catch problems before doInitialization() is
        // called on any passes.
        if (!NoVerify && verifyModule(*M, &errs())) {
            std::string Prefix =
                (Twine(arg0) + Twine(": ") + Twine(InputFilename)).str();

            WithColor::error(errs(), Prefix) << "input module is broken!\n";

            throw std::runtime_error("Module is broken");
        }

        // Override function attributes based on CPUStr, FeaturesStr, and command line
        // flags.
        setFunctionAttributes(CPUStr, FeaturesStr, *M);

        if (RelaxAll.getNumOccurrences() > 0 &&
            FT != TargetMachine::CGFT_ObjectFile)
            WithColor::warning(errs(), arg0)
                << ": warning: ignoring -mc-relax-all because filetype != obj";

        {

            // Manually do the buffering rather than using buffer_ostream,
            // so we can memcmp the contents in CompileTwice mode
            SmallVector<char, 0> Buffer;
            std::unique_ptr<raw_svector_ostream> BOS;

            if ((FT != TargetMachine::CGFT_AssemblyFile /*&&
                !outputStream.supportsSeeking()*/)) {
                BOS = std::make_unique<raw_svector_ostream>(Buffer);
                outputStream = BOS.get();
            }

            LLVMTargetMachine &LLVMTM = static_cast<LLVMTargetMachine &>(*Target);

            MachineModuleInfo *MMIWP =
                new MachineModuleInfo(&LLVMTM);

            std::unique_ptr<ToolOutputFile> DwoOut;

            raw_fd_ostream *OS;

            std::error_code EC;
            static raw_fd_ostream outss("-", EC, sys::fs::OF_None);

            assert(!EC);

            OS = &outss;

            if (Target->addPassesToEmitFile(PM, *OS,
                nullptr,
                FT, NoVerify, MMIWP)) {
                WithColor::warning(errs(), arg0)
                    << "target does not support generation of this"
                    << " file type!\n";

                throw std::runtime_error("Failed");
            }

            // Before executing passes, print the final values of the LLVM options.
            cl::PrintOptionValues();

            PM.run(*M);

//            auto HasError =
//                ((const DiagnosticHandler *)(llvmModule->getContext().getDiagHandlerPtr()))->HasError;
//
//            if (*HasError)
//                throw std::runtime_error("Has error");

            if (BOS) {
                *OS << Buffer;
            }
        }

        // Declare success.
//        Out->keep();
//
//        if (DwoOut)
//            DwoOut->keep();
    }

    bool Driver::makeObjectCode3(llvm::Triple targetTriple, llvm::Module *module) {
        // Initialize targets for emitting object code.
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        std::string error;

        const llvm::Target *target = llvm::TargetRegistry::lookupTarget(
            targetTriple.getTriple(),
            error
        );

        /**
         * The requested target could not be found. This might occur if
         * the target registry was not previously initialized, or if a
         * bogus target triple was provided.
         */
        if (!target) {
            log::error("Could not lookup target: " + error);

            return false;
        }

        std::string cpuName = llvm::sys::getHostCPUName();

        // Build CPU features.
        SubtargetFeatures subtargetFeatures = SubtargetFeatures();
        StringMap<bool> hostFeatures = StringMap<bool>();

        if (sys::getHostCPUFeatures(hostFeatures)) {
            for (auto &feature : hostFeatures) {
                subtargetFeatures.AddFeature(feature.first(), feature.second);
            }
        }

        std::string cpuFeatures = subtargetFeatures.getString();
        llvm::TargetOptions targetOptions = llvm::TargetOptions();

        llvm::Optional<llvm::Reloc::Model> relocationModel =
            llvm::Optional<llvm::Reloc::Model>();

        llvm::TargetMachine *targetMachine = target->createTargetMachine(
            targetTriple.getTriple(),
            cpuName,
            cpuFeatures,
            targetOptions,
            relocationModel
        );

        /**
         * Configure the module's data layout and target triple
         * for optimization benefits (performance). Optimizations
         * benefit from knowing about the target triple and data
         * layout.
         */
         // TODO: ->createDataLayout() is causing SIGSEGV.
//        llvmModule->setDataLayout(targetMachine->createDataLayout());
//        llvmModule->setTargetTriple(targetTriple.getTriple());

        std::error_code errorCode = std::error_code();

        llvm::raw_fd_ostream destination = llvm::raw_fd_ostream(
            this->outputFilePath.string(),
            errorCode,
            llvm::sys::fs::OF_None
        );

        if (errorCode) {
            log::error("Could not open output file: " + errorCode.message());

            return false;
        }

        llvm::legacy::PassManager passManager;

        llvm::TargetMachine::CodeGenFileType outputFileType =
            llvm::TargetMachine::CodeGenFileType::CGFT_ObjectFile;

        // NOTE: Returns true upon failure.
        bool failed = targetMachine->addPassesToEmitFile(
            passManager,
            destination,
            nullptr,
            outputFileType
        );

        if (failed) {
            log::error("LLVM cannot emit this type of file");

            return false;
        }

        passManager.run(*module);
        destination.flush();

        return true;
    }

    void Driver::tryThrow(std::exception exception) {
        if (cli::options.jitThrow) {
            throw exception;
        }
    }

    bool Driver::run(
        llvm::Triple targetTriple,
        std::filesystem::path outputFilePath,
        std::string input
    ) {
        this->outputFilePath = outputFilePath;
        this->input = input;

        std::vector<ionlang::Token> tokens = this->lex();

        ionshared::Ptr<DiagnosticVector> diagnostics =
            std::make_shared<DiagnosticVector>();

        ionshared::OptPtr<ionlang::Module> ionLangModules = this->parse(tokens, diagnostics);

        if (!ionshared::util::hasValue(ionLangModules)) {
            return false;
        }

        std::optional<std::vector<llvm::Module *>> llvmModules =
            this->lowerToLlvmIr(*ionLangModules, diagnostics);

        if (!llvmModules.has_value() || llvmModules->empty()) {
            return false;
        }

        // TODO: Processing only first module until implemented support for multiple (consider multiple modules inside a single file).
        return this->makeObjectCode3(targetTriple, llvmModules.value()[0]);
    }
}
