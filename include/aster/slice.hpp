#ifndef _ASTER_SLICE_HPP
#define _ASTER_SLICE_HPP

/// C++ Includes
#include <string_view>

namespace Aster {

    /// @brief Component Hints.
    enum class Hint : uint8_t {
        WILDCARD,  // '*'
        GLOBSTAR,  // '**'
        EXTENDS,   // '*.ext'

        SPECIAL,  // special value
        LITERAL,  // literal value
    };

    /// @brief Pattern Slice.
    class Slice {
        //  PROPERTIES  //

        /// @brief Associated hint.
        Hint m_hint = Hint::LITERAL;

        /// @brief The underlying slice.
        uint32_t m_size = 0;

        /// @brief Data-slice section.
        const char* m_slice = "";

       public:
        //  CONSTRUCTORS  //

        /// @brief Defaulted pattern component.
        constexpr Slice() = default;

        /**
         * @brief Constructs a pattern component.
         * @param slice             Slice view.
         * @param size              Size of slice.
         * @param hint              Optional hint.
         */
        constexpr Slice(const char* slice, uint32_t size) : m_size(size), m_slice(slice) {}
        constexpr Slice(const char* slice, uint32_t size, Hint hint) : m_hint(hint), m_size(size), m_slice(slice) {}

        /**
         * @brief Constructs a literal component.
         * @param view              Slice view.
         * @param hint              Optional hint.
         */
        constexpr Slice(const std::string_view& view) : Slice(view.data(), view.size()) {}
        constexpr Slice(const std::string_view& view, Hint hint) : Slice(view.data(), view.size(), hint) {}

        //  PUBLIC METHODS  //

        inline constexpr Hint hint() const noexcept { return m_hint; }
        inline constexpr size_t size() const noexcept { return m_size; }
        inline constexpr std::string_view view() const noexcept { return { m_slice, m_size }; }
    };

}  // namespace Aster

#endif
