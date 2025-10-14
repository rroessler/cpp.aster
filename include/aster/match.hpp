#ifndef _ASTER_MATCH_HPP
#define _ASTER_MATCH_HPP

/// C++ Includes
#include <algorithm>
#include <vector>

/// Aster Incldues
#include "aster/detail.hpp"

namespace Aster {

    /// @brief Handlers pattern matching.
    class Match {
        //  TYPEDEFS  //

        /// @brief Current processing mode.
        enum class Mode : uint8_t { DONE, OKAY, FAIL, WILD };

        /// @brief Available ASTER Actions.
        struct Action {
            static constexpr auto WILD_STAR = '*';
            static constexpr auto WILD_QUERY = '?';
            static constexpr auto WILD_NEGATE = '!';
            static constexpr auto WILD_ESCAPE = '\\';

            static constexpr auto BRACE_OPEN = '{';
            static constexpr auto BRACE_CLOSE = '}';
            static constexpr auto BRACE_COMMA = ',';

            static constexpr auto BRACK_OPEN = '[';
            static constexpr auto BRACK_CLOSE = ']';
            static constexpr auto BRACK_INVERT = '^';
        };  // namespace Action

        /// @brief Braces stack typing.
        using Stack = std::vector<std::pair<uint32_t, uint32_t>>;

        /// @brief Braces state holder.
        struct Braces {
            uint32_t depth = 0;
            uint32_t index = 0;
            uint32_t opened = 0;
            bool brackets = false;
        };

        /// @brief Matcher Wildcard Properties.
        struct Wildcard {
            uint32_t path = 0;
            uint32_t glob = 0;
            uint32_t braces = 0;
        };

        //  PROPERTIES  //

        Wildcard m_state = {};     // Baseline matching state.
        Wildcard m_asterisk = {};  // Singular '*' glob pattern.
        Wildcard m_globstar = {};  // Double '**' globstar pattern.

        /// @brief The pending braces stack.
        Stack* m_pending = nullptr;

        //  CONSTRUCTORS  //

        /// @brief Constructs a matcher instance.
        constexpr Match() = default;

        /**
         * @brief Constructs a matcher instance.
         * @param pending           Pending braces.
         */
        constexpr Match(Stack* pending) : m_pending(pending) {}

       public:
        //  PUBLIC METHODS  //

        /**
         * @brief Handles matching globbing patterns.
         * @param glob              Pattern to consume.
         * @param input             Input to validate.
         */
        static inline constexpr bool glob(std::string_view glob, const std::string_view& input) {
            // ignore if the glob immediately fails at all
            if (glob.empty()) return input.empty();

            // prepare the details to be used
            auto pending = Stack();
            auto matcher = Match(&pending);

            // get the incoming negation to be used
            auto negated = matcher.m_negate_pattern(glob);

            // and attempt matching the pattern now
            return negated ^ matcher.m_matches_pattern(glob, input, 0);
        }

        /**
         * @brief Handles matching empty patterns.
         * @param glob              Ignored pattern.
         * @param input             Input to validate.
         */
        static inline constexpr bool empty(std::string_view, const std::string_view& input) { return input.empty(); }

        /**
         * @brief Checks for exact matches.
         * @param glob              Pattern to consume.
         * @param input             Input to match.
         */
        static inline constexpr bool exact(std::string_view glob, const std::string_view& input) {
            return Match().m_matches_exact(glob, input);
        }

        /**
         * @brief Checks for trailing matches.
         * @param glob              Pattern to consume.
         * @param input             Input to match.
         */
        static inline constexpr bool extends(std::string_view glob, const std::string_view& input) {
            return input.ends_with(glob.substr(glob.find_last_of('*') + 1));
        }

       private:
        //  PRIVATE METHODS  //

        /**
         * @brief Gets the expected negation of a pattern.
         * @param glob              Pattern to negate.
         */
        inline constexpr bool m_negate_pattern(std::string_view& glob) const noexcept {
            auto index = glob.find_first_not_of(Action::WILD_NEGATE);
            return glob.remove_prefix(index), index % 2;  // and resolve
        }

        /**
         * @brief Gets the expected negation of a bracket.
         * @param slice             Bracket slice.
         */
        inline constexpr bool m_negate_bracket(const std::string_view& slice) noexcept {
            if (slice.find_first_of("!^")) return false;
            return m_state.glob += 1, true;  // negated
        }

        /**
         * @brief Checks for exact matches.
         * @param glob              Pattern to consume.
         * @param input             Input to validate.
         */
        inline constexpr bool m_matches_exact(const std::string_view& glob, const std::string_view& input) {
            // attempt iterating whilst we still have input/glob to process
            for (bool pending = false; (pending = m_state.glob < glob.size()) || m_state.path < input.size();) {
                // we failed as we still have some glob-pattern left
                if (!pending) return false;

                // otherwise check against incoming glob characters
                auto ch = glob[m_state.glob];

                // fail if possible here
                if (m_process_character(ch, glob, input) > Mode::OKAY) return false;
            }

            // if we reach here, then both the input/glob has been completed
            return true;
        }

        /**
         * @brief Handles matching an input with a glob pattern.
         * @param glob              Pattern to consume.
         * @param input             Input to validate.
         * @param start             The starting position.
         */
        inline constexpr bool m_matches_pattern(
            const std::string_view& glob, const std::string_view& input, uint32_t start) {
            // attempt iterating whilst we still have input/glob to process
            for (bool pending = false; (pending = m_state.glob < glob.size()) || m_state.path < input.size();) {
                // get the incoming mode to be handled now
                auto mode = pending ? m_process_pattern(glob, input, start) : Mode::WILD;

                // handle the incoming mode as necessary
                switch (mode) {
                    // could not match so we allow matching wildcards
                    case Mode::WILD: {
                        if (!m_asterisk.path || m_asterisk.path > input.size()) return false;
                        m_state = m_asterisk;  // we can safely reset the state with the wildcard
                    } break;

                    // immediate match so we can safely break
                    case Mode::OKAY: break;

                    // should immediately declare as a failure
                    case Mode::FAIL: return false;

                    // should immediately declare as a success
                    case Mode::DONE: return true;
                }
            }

            // if we reach here, then both the input/glob has been completed
            return true;
        }

        /**
         * @brief Handles matching brace expansions.
         * @param glob              Glob to expand.
         * @param input             Incoming input.
         * @param braces            Brace expansion.
         */
        inline constexpr bool m_matches_braces(
            const std::string_view& glob, const std::string_view& input, Braces& braces) {
            // push the pending braces details
            m_pending->emplace_back(braces.opened, braces.index);

            // clone the current matcher to be used
            auto cloned = Match(*this);
            cloned.m_state.glob = braces.index;
            cloned.m_state.braces = m_pending->size();

            auto result = cloned.m_matches_pattern(glob, input, braces.index);
            return m_pending->pop_back(), result;  // and resolve now as needed
        }

        /**
         * @brief Handles brace expansions.
         * @param glob              Glob to expand.
         * @param input             Incoming input.
         * @param braces            Brace expansion.
         */
        inline constexpr bool m_expand_braces(
            const std::string_view& glob, const std::string_view& input, Braces braces) {
            // initialize the open-brances index
            braces.opened = m_state.glob;

            // attempt expanding all our braces now
            for (; m_state.glob < glob.size(); m_state.glob += 1) {
                switch (glob[m_state.glob]) {
                    case Action::BRACE_OPEN: {
                        if (braces.brackets) break;  // skip if in brackets
                        else if ((braces.depth += 1) == 1) braces.index = m_state.glob + 1;
                    } break;

                    case Action::BRACE_CLOSE: {
                        if (braces.brackets) break;
                        if ((braces.depth -= 1) != 0) continue;
                        if (m_matches_braces(glob, input, braces)) return true;
                    } break;

                    case Action::BRACE_COMMA: {
                        if (braces.depth != 1) break;  // ignore here
                        if (m_matches_braces(glob, input, braces)) return true;
                        braces.index = m_state.glob + 1;  // check next item
                    } break;

                    case Action::BRACK_OPEN: braces.brackets = true; break;
                    case Action::BRACK_CLOSE: braces.brackets = false; break;
                    case Action::WILD_NEGATE: m_state.glob += 1; break;
                    default: break;  // ignore basic items
                }
            }

            // if we get here then we fail
            return false;
        }

        /**
         * @brief Handles skipping separators.
         * @param input             Input to skip.
         * @param invalid           Invalid pattern end.
         */
        inline constexpr void m_skip_to_separator(const std::string_view& input, bool invalid) {
            // if at the end of the input, then we can increment once
            if (m_state.path == input.size()) return void(m_asterisk.path += 1);

            // consume the incoming input as possible
            auto index = std::min(input.find_first_of(Detail::separator(), m_state.path), input.size());

            // update the path index depending on the validness
            m_asterisk.path = index + (invalid || index != input.size());

            // update the current globstar now
            m_globstar = m_asterisk;
        }

        /**
         * @brief Handles skipping over branches.
         * @param glob              Incoming glob pattern.
         * @param brackets          Brackets sequence flag.
         */
        inline constexpr Mode m_skip_over_branch(const std::string_view& glob, bool brackets = false) {
            for (uint32_t ending = m_state.braces - 1; m_state.glob < glob.size(); m_state.glob += 1) {
                switch (glob[m_state.glob]) {
                    case Action::BRACE_OPEN: m_state.braces += !brackets; break;
                    case Action::BRACE_CLOSE: {
                        if (brackets) break;  // ignore in brackets
                        if ((m_state.braces -= 1) != ending) break;
                        return m_state.glob += 1, Mode::OKAY;
                    }

                    case Action::BRACK_OPEN: brackets = true; break;
                    case Action::BRACK_CLOSE: brackets = false; break;
                    case Action::WILD_ESCAPE: m_state.glob += 1; break;
                    default: break;  // regular characters found here
                }
            }

            // should be handled normally again now
            return Mode::OKAY;
        }

        /**
         * @brief Handles escaping characters.
         * @param glob              Incoming glob pattern.
         * @param ch                Character to escape.
         */
        inline constexpr bool m_unescape(const std::string_view& glob, char& ch) {
            // immediately ignore if the character is valid
            if (ch != '\\') return true;

            // update the incoming index now as necessary
            if ((m_state.glob += 1) >= glob.size()) return false;

            // handle the next available character now
            switch (ch = glob[m_state.glob]) {
                case 'a': ch = '\x61'; break;
                case 'b': ch = '\x08'; break;
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;

                // valid character so ignore updating
                default: break;
            }

            // declare as validly escaped now
            return ch;
        }

        /**
         * @brief Handles processing specific matches.
         * @param glob                  Pattern to consume.
         * @param input                 Input to match.
         * @param start                 The starting position.
         */
        template <char A>
        Mode m_process_action(const std::string_view& glob, const std::string_view& input, uint32_t start);

        /**
         * @brief Handles processing generic matches.
         * @param glob                  Pattern to consume.
         * @param input                 Input to match.
         * @param start                 The starting position.
         */
        inline constexpr Mode m_process_pattern(
            const std::string_view& glob, const std::string_view& input, uint32_t start) {
            // prepare the incoming character to be processed
            auto ch = glob[m_state.glob];

            // check for any specific items to be processed
            switch (ch) {
                case Action::WILD_STAR: return m_process_action<Action::WILD_STAR>(glob, input, start);
                case Action::WILD_QUERY: return m_process_action<Action::WILD_QUERY>(glob, input, start);
                case Action::BRACK_OPEN: return m_process_action<Action::BRACK_OPEN>(glob, input, start);
                case Action::BRACE_OPEN: return m_process_action<Action::BRACE_OPEN>(glob, input, start);

                // both the closing and commas for braces should be skippable
                case Action::BRACE_CLOSE:
                case Action::BRACE_COMMA: {
                    if (m_state.braces) return m_skip_over_branch(glob);
                    return m_process_character(ch, glob, input);
                }

                // otherwise handle as a regular character now
                default: return m_process_character(ch, glob, input);
            }
        }

        /**
         * @brief Handles processing character values.
         * @param ch                    Character to process.
         * @param glob                  Pattern to consume.
         * @param input                 Input to be validated.
         */
        inline constexpr Mode m_process_character(
            char ch, const std::string_view& glob, const std::string_view& input) {
            // ensure we actually have a full input
            if (m_state.path >= input.size()) return Mode::WILD;

            // check if we cannot escape the incoming character at all
            if (!m_unescape(glob, ch)) return Mode::FAIL;

            // check if the incoming item is valid now
            auto separator = ch == '/' && Detail::separator(input[m_state.path]);
            if (!separator && input[m_state.path] != ch) return Mode::WILD;

            // have a suitably match, so we want to update now
            m_state.glob += 1, m_state.path += 1;

            // and update the wildcard if necessary
            if (ch == '/') m_asterisk = m_globstar;

            // declare as currently happy
            return Mode::OKAY;
        }

        //  SPECIALIZATIONS  //

        /// @brief Handles incoming '*' and '**' characters.
        template <>
        inline constexpr Mode m_process_action<Action::WILD_STAR>(
            const std::string_view& glob, const std::string_view& input, uint32_t start) {
            // check if we have an incoming globstar '**' at all
            auto globstar = glob.substr(m_state.glob).starts_with("**");

            // skip over globstars if necessary to do so
            if (globstar) {
                uint32_t index = m_state.glob + 2;  // result index
                while (glob.substr(index, 3) == "/**") index += 3;
                m_state.glob = index - 2;  // and resolve index now
            }

            // update the current aster details now
            m_asterisk = m_state, m_asterisk.path += 1;

            // update the current globbing position
            m_state.glob += 1 + globstar;

            // denote the current recursive state of globstars
            bool recursive = false;

            // handle any incoming globstars now
            if (globstar) {
                auto invalid = m_state.glob != glob.size();  // check if invalid ending
                auto separator = m_state.glob - start < 3 || glob[m_state.glob - 3] == '/';
                separator = separator && (!invalid || glob[m_state.glob] == '/');
                if (separator) m_state.glob += invalid, m_skip_to_separator(input, invalid), recursive = true;
            }

            auto collapse = !recursive && m_state.path < input.size() && Detail::separator(input[m_state.path]);
            if (collapse) m_asterisk = m_globstar;  // should be able to collapse the globstar now

            // declare as a success now
            return Mode::OKAY;
        }

        /// @brief Handles incoming '?' characters.
        template <>
        inline constexpr Mode m_process_action<Action::WILD_QUERY>(
            const std::string_view&, const std::string_view& input, uint32_t) {
            if (Detail::separator(input[m_state.path])) return Mode::WILD;
            return m_state.glob += 1, m_state.path += 1, Mode::OKAY;
        }

        /// @brief Handles incoming '[...]' matches
        template <>
        inline constexpr Mode m_process_action<Action::BRACK_OPEN>(
            const std::string_view& glob, const std::string_view& input, uint32_t) {
            // ignore if the input has been completed at all
            if (m_state.path >= input.size()) return Mode::WILD;

            // ensure we increment the glob details now
            m_state.glob += 1;

            // prepare the final matching state
            bool matched = false;

            // prepare the incoming input-character now
            auto ch = input[m_state.path];

            // get the slice and the negation state
            auto negated = m_negate_bracket(glob.substr(m_state.glob));

            // attempt eating our possible incoming set
            for (bool first = true; m_state.glob < glob.size() && (first || glob[m_state.glob] != ']'); first = false) {
                auto low = glob[m_state.glob];  // get low-value
                if (!m_unescape(glob, low)) return Mode::FAIL;

                // increment the current character count now
                m_state.glob += 1;

                // prepare the high-value to be used now
                auto high = low;

                // only set high if necessary (eg: we have an opening '-' and no closing bracket)
                if (m_state.glob + 1 < glob.size() && glob[m_state.glob] == '-' && glob[m_state.glob + 1] != ']') {
                    m_state.glob += 1, high = glob[m_state.glob];
                    if (!m_unescape(glob, high)) return Mode::FAIL;
                    m_state.glob += 1;  // valid so increment size
                }

                // check for a suitably match now if possible
                if (low <= ch && ch <= high) matched = true;
            }

            // we were able to suitably match our closing bracket
            if (m_state.glob >= glob.size()) return Mode::FAIL;

            // ensure we always increment the globbing index now
            m_state.glob += 1;

            // otherwise handle based on the matched state
            return matched != negated ? m_state.path += 1, Mode::OKAY : Mode::WILD;
        }

        /// @brief Handles incoming '{...}' expansion.
        template <>
        inline constexpr Mode m_process_action<Action::BRACE_OPEN>(
            const std::string_view& glob, const std::string_view& input, uint32_t) {
            auto predicate = [this](const auto& pair) { return pair.first == m_state.glob; };
            auto found = std::ranges::find_if(*m_pending, predicate);  // attempt matching

            if (found == m_pending->cend()) return m_expand_braces(glob, input, {}) ? Mode::DONE : Mode::FAIL;
            else return m_state.glob = found->second, m_state.braces += 1, Mode::OKAY;  // always valid here
        }
    };

}  // namespace Aster

#endif
