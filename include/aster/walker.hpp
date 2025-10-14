#ifndef _ASTER_WALKER_HPP
#define _ASTER_WALKER_HPP

/// Aster Includes
#include "aster/iterator.hpp"

namespace Aster {

    /// @brief Explicit Walker Instance.
    class Walker {
        //  PROPERTIES  //

        /// @brief Walker pattern value.
        Pattern m_pattern = Pattern("**/*");

       public:
        //  CONSTRUCTORS  //

        /// @brief Constructs a recursive walker.
        constexpr Walker() = default;

        /**
         * @brief Constructs a walker instance.
         * @param pattern               Pattern to bind.
         */
        constexpr Walker(const Pattern& pattern) : m_pattern(pattern) {}

        //  PUBLIC METHODS  //

        /// @brief Gets the underlying walker pattern.
        inline constexpr const Pattern& pattern() const noexcept { return m_pattern; }

        /**
         * @brief Initiates an iteration sequence.
         * @param options               Iteration options.
         */
        inline constexpr Iterator iterate(const Options& options = {}) const noexcept {
            return ++Iterator(&m_pattern, options);  // construct the iterator now
        }
    };

}  // namespace Aster

#endif
