#pragma once

#include <ilc/misc/helpers.h>

namespace ilc {
    class Util {
    public:
        template<typename T>
        static bool hasValue(OptPtr<T> value) {
            return value.has_value() && *value != nullptr;
        }
    };
}
