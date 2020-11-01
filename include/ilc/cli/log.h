#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <ilc/cli/options.h>
#include <ilc/cli/console_color.h>

namespace ilc::log {
    enum struct LogLevel {
        Verbose = (int)ColorKind::ForegroundGray,

        Success = (int)ColorKind::ForegroundGreen,

        Info = (int)ColorKind::ForegroundCyan,

        Warning = (int)ColorKind::ForegroundYellow,

        Error = (int)ColorKind::ForegroundRed,

        // TODO: Better color for fatal errors.
        Fatal = (int)ColorKind::ForegroundWhite,

        Debug = (int)ColorKind::ForegroundMagenta
    };

    static std::optional<std::string> findLogLevelText(LogLevel logLevel) {
        switch (logLevel) {
            case LogLevel::Verbose: {
                return "verbose";
            }

            case LogLevel::Success: {
                return "ok";
            }

            case LogLevel::Info: {
                return "info";
            }

            case LogLevel::Warning: {
                return "warning";
            }

            case LogLevel::Error: {
                return "error";
            }

            case LogLevel::Fatal: {
                return "fatal";
            }

            case LogLevel::Debug: {
                return "debug";
            }

            default: {
                return std::nullopt;
            }
        }
    }

    [[nodiscard]] static std::stringstream makeLogTemplate(
        LogLevel logLevel,
        ColorKind color,
        std::string text
    ) {
        std::optional<std::string> logLevelText = log::findLogLevelText(logLevel);

        if (!logLevelText.has_value()) {
            throw std::runtime_error("Could not determine text of provided log level");
        }

        std::stringstream resultStream{};

        resultStream << ConsoleColor::coat(*logLevelText, color)
            << " "
            << text
            << "\n";

        return resultStream;
    }

    static void makeAndPrint(LogLevel logLevel, std::string text) {
        std::cout << log::makeLogTemplate(logLevel, (ColorKind)logLevel, text).str();
    }

    static void verbose(std::string text) {
        if (cli::options.noVerbose) {
            return;
        }

        log::makeAndPrint(LogLevel::Verbose, text);
    }

    static void success(std::string text) {
        log::makeAndPrint(LogLevel::Success, text);
    }

    static void info(std::string text) {
        log::makeAndPrint(LogLevel::Info, text);
    }

    static void warning(std::string text) {
        log::makeAndPrint(LogLevel::Warning, text);
    }

    static void error(std::string text) {
        log::makeAndPrint(LogLevel::Error, text);
    }

    static void fatal(std::string text) {
        log::makeAndPrint(LogLevel::Fatal, text);
    }

    static void debug(std::string text) {
        if (!cli::options.doDebug) {
            return;
        }

        log::makeAndPrint(LogLevel::Debug, text);
    }
}
