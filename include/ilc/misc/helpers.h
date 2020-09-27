#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <ionshared/diagnostics/diagnostic.h>
#include <ionshared/container/vector.h>

namespace ilc {
    typedef void (*Callback)();

    template<typename T>
    using Ptr = std::shared_ptr<T>;

    template<typename T>
    using OptPtr = std::optional<Ptr<T>>;

    typedef ionshared::Vector<ionshared::Diagnostic> DiagnosticVector;
}
