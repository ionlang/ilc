#include <string>
#include <stdexcept>
#include <iostream>
#include <ilc/cli/options.h>
#include <ilc/cli/console_color.h>

namespace ilc::log {
    enum class LogLevel {
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
                return "Verbose";
            }

            case LogLevel::Success: {
                return "Success";
            }

            case LogLevel::Info: {
                return "Info";
            }

            case LogLevel::Warning: {
                return "Warning";
            }

            case LogLevel::Error: {
                return "Error";
            }

            case LogLevel::Fatal: {
                return "Fatal";
            }

            case LogLevel::Debug: {
                return "Debug";
            }

            default: {
                return std::nullopt;
            }
        }
    }

    static void make(LogLevel logLevel, std::string text) {
        std::optional<std::string> logLevelText = log::findLogLevelText(logLevel);

        if (!logLevelText.has_value()) {
            throw std::runtime_error("Could not determine text of provided log level");
        }

        std::cout << ConsoleColor::coat("[", ColorKind::ForegroundGray)
            << ConsoleColor::coat(*logLevelText, (ColorKind)logLevel)
            << ConsoleColor::coat("] ", ColorKind::ForegroundGray)
            << text
            << std::endl;
    }

    static void verbose(std::string text) {
        if (cli::options.noVerbose) {
            return;
        }

        make(LogLevel::Verbose, text);
    }

    static void success(std::string text) {
        log::make(LogLevel::Success, text);
    }

    static void info(std::string text) {
        log::make(LogLevel::Info, text);
    }

    static void warning(std::string text) {
        log::make(LogLevel::Warning, text);
    }

    static void error(std::string text) {
        log::make(LogLevel::Error, text);
    }

    static void fatal(std::string text) {
        log::make(LogLevel::Fatal, text);
    }

    static void debug(std::string text) {
        if (!cli::options.doDebug) {
            return;
        }

        log::make(LogLevel::Debug, text);
    }
}
