#pragma once

#include <functional>
#include <memory>
#include <optional>

namespace ilc {
    typedef void (*Callback)();

    template<typename T>
    using Ptr = std::shared_ptr<T>;

    template<typename T>
    using OptPtr = std::optional<Ptr<T>>;
}
