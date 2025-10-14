#ifndef _ASTER_ITERATOR_HPP
#define _ASTER_ITERATOR_HPP

/// C++ Includes
#include <iterator>
#include <memory>

/// Aster Includes
#include "aster/pattern.hpp"

/// OS Includes
#include "aster/unix/traversal.hpp"
#include "aster/win32/traversal.hpp"

namespace Aster {

    /// @brief Traversal Options.
    struct Options {
        //  PROPERTIES  //

        bool files = true;         // Allow matching files.
        bool hidden = false;       // Allow matching hidden.
        bool symlinks = false;     // Allow matching symlinks.
        bool directories = false;  // Allow matching directories.

        /// @brief The current working directory.
        std::string cwd = Detail::getcwd();
    };

    /// @brief Glob Pattern Iterator.
    class Iterator {
        //  PROPERTIES  //

        /// @brief Iterator options.
        Options m_options = Options();

        /// @brief Associated glob pattern.
        const Pattern* m_pattern = nullptr;

        /// @brief The pending directories queue.
        std::vector<std::string> m_pending = {};

        /// @brief Encapsulated traversal implementation.
        std::shared_ptr<Detail::Traversal> m_traversal = nullptr;

       public:
        //  CONSTRUCTORS  //

        /// @brief Constructs a sentinel iterator value.
        constexpr Iterator(std::default_sentinel_t) {}

        /**
         * @brief Constructs a glob-iterator.
         * @param options               Iterator options.
         */
        constexpr Iterator(const Options& options = {}) : Iterator(m_dynamic(), options) {}

        /**
         * @brief Constructs a glob-iterator.
         * @param pattern               Glob pattern.
         * @param options               Iterator options.
         */
        constexpr Iterator(const Pattern* pattern, const Options& options = Options()) :
            m_pattern(pattern), m_pending({ options.cwd }) {}

        //  OPERATOR METHODS  //

        /// @brief Gets the current iterator value.
        inline constexpr Entry operator*() const noexcept { return m_traversal ? m_traversal->current() : Entry(); }

        /// @brief Gets the current iterator reference.
        inline constexpr const Entry* operator->() const noexcept {
            return m_traversal ? &m_traversal->current() : nullptr;
        }

        /// @brief Handles advancing the iterator.
        inline constexpr Iterator& operator++() { return m_advance(), *this; }

        /// @brief Compares two iterator values if equal.
        inline constexpr bool operator==(const Iterator& other) const noexcept {
            return m_pending.empty() && m_traversal == other.m_traversal;
        }

        //  PUBLIC METHODS  //

        inline constexpr Iterator begin() const noexcept { return *this; }
        inline constexpr Iterator end() const noexcept { return Iterator(std::default_sentinel); }

       private:
        //  PRIVATE METHODS  //

        /// @brief Handles advancing the iterator.
        inline constexpr void m_advance() {
            // stop if there is no traversal handler
            if (!m_prime()) return;

            // attempt scanning whilst possible to do so
            while (!m_traversal->done()) {
                auto entry = m_traversal->advance();  // next
                if (entry.path.size() && m_test(entry)) return;
            }

            // if we reach here, clear the traversal now
            m_traversal = nullptr;

            // tail-call into advancing further now
            [[clang::musttail]] return m_advance();
        }

        /// @brief Handles priming the traversal handler.
        inline constexpr bool m_prime() {
            // if already primed, then
            if (m_traversal != nullptr) return true;

            // check if we have a pending value
            if (m_pending.empty()) return false;

            // construct the next traversal
            m_traversal = std::make_shared<Detail::Traversal>(m_pending.back());

            // and declare as still running now
            return m_pending.pop_back(), true;
        }

        /**
         * @brief Handles testing incoming entries.
         * @param entry                 Traversal entry.
         */
        inline constexpr bool m_test(const Entry& entry) {
            // hande the incoming entry typing
            switch (entry.type) {
                case Archetype::REGULAR: return m_options.files && m_test(entry.path);
                case Archetype::SYMLINK: return m_options.symlinks && m_test(entry.path);
                case Archetype::DIRECTORY: break;
                default: return false;
            }

            // for directories we want to push when the pattern is recursive
            if (m_pattern->recursive()) m_pending.emplace_back(entry.path);

            // and match only if directories can be matched
            return m_options.directories && m_test(entry.path);
        }

        /**
         * @brief Handles testing incoming paths.
         * @param input                 Input path.
         */
        inline constexpr bool m_test(std::string_view input) const noexcept {
            if (!m_pattern->absolute()) input.remove_prefix(m_options.cwd.size() + 1);
            return m_pattern->matches(input);  // check if the input matches now
        }

        /// @brief Gets the underlying dynamic pattern.
        static inline constexpr const Pattern* m_dynamic() noexcept {
            static auto s_dynamic = Pattern("**/*");
            return &s_dynamic;  // get the pattern
        }
    };

    /// @brief Ending Iterator.
    struct Sentinel : public Iterator {
        //  CONSTRUCTORS  //

        /// @brief Constructs a sentinel iterator.
        explicit constexpr Sentinel() : Iterator(std::default_sentinel) {}
    };

}  // namespace Aster

#endif
