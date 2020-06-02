#pragma once

#include <optional>
#include <string>

namespace ilc {
    class FileSystem {
    public:
        static bool doesPathExist(const std::string &name);

        static std::optional<std::string> readFileContents(std::string path);
    };
}
