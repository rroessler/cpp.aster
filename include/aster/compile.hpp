#ifndef _ASTER_COMPILER_HPP
#define _ASTER_COMPILER_HPP

/// C++ Includes
#include <span>
#include <vector>

/// Aster Includes
#include "aster/detail.hpp"
#include "aster/match.hpp"
#include "aster/slice.hpp"

namespace Aster::Detail {

    /// @brief Available Pattern Flags.
    struct Flags {
        bool negated : 1 = false;
        bool globstar : 1 = false;
        bool absolute : 1 = false;
        bool exact : 1 = false;
    };

    /// @brief Pattern Matching Algorithm.
    using Algorithm = bool (*)(std::string_view glob, const std::string_view& input);

    /// @brief Encoded Pattern Components.
    using Encoded = std::tuple<std::string, std::vector<Slice>, Algorithm, Flags>;

    /// @brief Glob Pattern Compiler.
    class Compile {
        //  TYPEDEFS  //

        /// @brief Compilation State.
        struct State {
            //  PROPERTIES  //

            /// @brief Starting index.
            uint32_t start = 0;

            /// @brief Current index.
            uint32_t index = 0;

            /// @brief Currently compiled slices.
            std::vector<Slice> slices = {};

            //  CONSTRUCTORS  //

            /// @brief Default state constructor.
            constexpr State() = default;

            /**
             * @brief Starts the state at an index.
             * @param offset            Offset to jump.
             */
            constexpr State(uint32_t offset) : start(offset), index(offset) {}
        };

        //  PROPERTIES  //

        /// @brief Encapsulates an empty pattern.
        Encoded m_empty = { "", {}, Match::empty, Flags() };

       public:
        //  PUBLIC METHODS  //

        /**
         * @brief Compiles a set of pattern slices.
         * @param glob                  Glob to split.
         */
        inline constexpr Encoded pattern(const std::string_view& glob) const noexcept {
            // handle empty globs immediately
            if (glob.empty()) return m_empty;

            // get the baseline negation
            auto negation = glob.find_first_not_of('!');
            auto prefix = std::string(glob.substr(negation));

            // processes the baseline slices
            auto slices = m_process(State(), prefix);

            // prepare the flags to be used now
            auto flags = m_flags(prefix, slices, negation % 2);

            // handles deciding a suitable matching algorithm
            auto algorithm = m_algorithm(glob, slices, flags);

            // and return the final enoded result
            return { prefix, slices, algorithm, flags };
        }

       private:
        //  PRIVATE METHODS  //

        /**
         * @brief Handles deducing flags.
         * @param glob                  Glob pattern to deduce.
         * @param slices                Associated slices of glob.
         * @param negated               Current negation state.
         */
        inline constexpr Flags m_flags(
            const std::string_view& glob, const std::span<Slice>& slices, bool negated) const noexcept {
            // prepare some callbacks to deduce flags
            auto literal = [](const Slice& slice) { return slice.hint() == Hint::LITERAL; };

            // construct the resulting flags to be used now
            return {
                .negated = negated,
                .globstar = m_globstar(slices),
                .absolute = Detail::absolute(glob),
                .exact = std::ranges::all_of(slices, literal),
            };
        }

        /**
         * @brief Handles deducing algorithms.
         * @param glob                  Glob pattern to deduce.
         * @param slices                Associated slices of glob.
         * @param flags                 Flags to help deductions.
         */
        inline constexpr Algorithm m_algorithm(
            const std::string_view& glob, const std::span<Slice>& slices, const Flags& flags = {}) const noexcept {
            // resolve a baseline empty match handler for patterns
            if (glob.empty()) return Match::empty;

            // if we have literal slices only, then expect an exact match
            if (flags.exact) return Match::exact;

            // since contains only "**" and maybe "*", then becomes a passthrough
            if (flags.globstar) return [](auto, const auto&) { return true; };

            // allow testing for fast "extension" matches now
            if (auto size = slices.size(); size && slices.back().hint() == Hint::EXTENDS) {
                if (size == 1 || m_globstar(slices.subspan(0, size - 1))) return Match::extends;
            }

            // otherwise default to the baseline matcher
            return Match::glob;
        }

        /**
         * @brief Tests for globstar only sequences.
         * @param slices                Slices to validate.
         * @param globstar              The final result.
         */
        inline constexpr bool m_globstar(const std::span<Slice>& slices, bool globstar = false) const noexcept {
            for (const auto& slice : slices) {
                switch (slice.hint()) {
                    case Hint::WILDCARD: continue;  // bypass now
                    case Hint::GLOBSTAR: globstar = true; break;
                    default: return false;  // invalid value
                }
            }

            // and return the resulting globstar details
            return globstar;
        }

        /**
         * @brief Handles processing globs.
         * @param state                 Compilation state.
         * @param glob                  Glob pattern to read.
         */
        inline constexpr std::vector<Slice> m_process(State state, const std::string_view& glob) const noexcept {
            while (state.index < glob.size()) m_advance(state, glob);
            return m_emplace(state, glob), state.slices;
        }

        /**
         * @brief Handles advancing the glob stream.
         * @param state                 Compilation state.
         * @param glob                  Glob pattern given.
         */
        inline constexpr void m_advance(State& state, const std::string_view& glob) const noexcept {
            if (Detail::separator(glob[state.index++])) m_emplace(state, glob);
        }

        /**
         * @brief Handles emplacing a slice.
         * @param state                 Compilation state.
         * @param glob                  Glob pattern given.
         */
        inline constexpr void m_emplace(State& state, const std::string_view& glob) const noexcept {
            // prepare the offset and size of the component
            auto size = state.index - state.start;

            // get the incoming details about the pattern
            auto view = glob.substr(state.start, size);

            // remove any trailing separators that may be available
            if (Detail::separator().contains(view.back())) view.remove_suffix(1);

            // update the current starting point now
            state.start = state.index;

            // stop the the incoming size is currently empty (except for leading '/' values)
            if (state.slices.size() && view.empty()) return;

            // categories the incoming view now
            auto hint = m_categorize(view);

            // update the view if necessary (specifically for "*." hints)
            if (hint == Hint::EXTENDS) size -= 2, view.remove_prefix(2);

            // and categorize the incoming view now to emplace
            state.slices.emplace_back(view, hint);
        }

        /**
         * @brief Handles categorizing a slice.
         * @param slice                 Slice to categorize.
         */
        inline constexpr Hint m_categorize(const std::string_view& slice) const noexcept {
            // check for some simple immediate matches
            if (slice == Detail::wildcard()) return Hint::WILDCARD;
            if (slice == Detail::globstar()) return Hint::GLOBSTAR;

            // check if a possible extension can occur or for special chars
            auto extends = slice.starts_with(Detail::extends());

            // handle any special instances now
            auto special = Detail::special(slice.substr(extends ? 2 : 0));

            // handle our final result as necessary now
            return special ? Hint::SPECIAL : extends ? Hint::EXTENDS : Hint::LITERAL;
        }
    };

};  // namespace Aster::Detail

#endif
