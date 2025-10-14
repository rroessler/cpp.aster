#ifndef _ASTER_PATTERN_HPP
#define _ASTER_PATTERN_HPP

/// Aster Includes
#include "aster/compile.hpp"

namespace Aster {

    /// @brief Glob Pattern Container.
    class Pattern {
        //  PROPERTIES  //

        /// @brief The compiled pattern instance.
        Detail::Encoded m_encoded = { "", {}, Match::empty, Detail::Flags() };

       public:
        //  CONSTRUCTORS  //

        /// @brief Allow default construction of an empty pattern.
        constexpr Pattern() = default;

        /**
         * @brief Constructs a compiled pattern.
         * @param glob              Glob to compile.
         */
        constexpr Pattern(const char* glob) : Pattern(std::string(glob)) {}
        constexpr Pattern(const std::string_view& glob) : Pattern(std::string(glob)) {}
        constexpr Pattern(const std::string& glob) : m_encoded(Detail::Compile().pattern(glob)) {}

        //  PUBLIC METHODS  //

        /// @brief Denotes if the associated pattern is empty.
        inline constexpr bool empty() const noexcept { return m_buffer().empty(); }

        /// @brief Checks for incoming negation.
        inline constexpr bool negated() const noexcept { return m_flags().negated; }

        /// @brief Denotes if the pattern has a leading "absolute" path.
        inline constexpr bool absolute() const noexcept { return m_flags().absolute; }

        /// @brief Denotes if a pattern will always succeed matching.
        inline constexpr bool globstar() const noexcept { return m_flags().globstar; }

        /// @brief Denotes if the instance is recursive or not.
        inline constexpr bool recursive() const noexcept { return m_slices().size() > 1 || globstar(); }

        /// @brief Gets the glob pattern encapsulated.
        inline constexpr std::string_view view() const noexcept { return m_buffer(); }

        /// @brief Gets the component slices of the pattern.
        inline constexpr std::span<const Slice> slices() const noexcept { return m_slices(); }

        /**
         * @brief Handles matching against this pattern.
         * @param input             Input to validate.
         */
        inline constexpr bool matches(const std::string_view& input) const noexcept {
            return m_flags().negated != m_algorithm()(m_buffer(), input);
        }

       private:
        //  PRIVATE METHODS  //

        inline constexpr std::string_view m_buffer() const noexcept { return std::get<0>(m_encoded); }
        inline constexpr const Detail::Flags& m_flags() const noexcept { return std::get<3>(m_encoded); }
        inline constexpr const std::vector<Slice>& m_slices() const noexcept { return std::get<1>(m_encoded); }
        inline constexpr const Detail::Algorithm& m_algorithm() const noexcept { return std::get<2>(m_encoded); }
    };

}  // namespace Aster

#endif
