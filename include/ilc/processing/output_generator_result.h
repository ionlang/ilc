#pragma once

namespace ilc {
    struct OutputGeneratorResult {
        bool success;

        /**
         * Verifies that the output files exist.
         */
        [[nodiscard]] bool verify();
    };
}
