#ifndef _ASTER_TRAVERSAL_HPP
#define _ASTER_TRAVERSAL_HPP

/// Aster Includes
#include "aster/entry.hpp"

#ifdef _ASTER_PLATFORM_UNIX

/// OS Includes
#include <dirent.h>

namespace Aster::Detail {

    /// @brief Directory Traversal Implementation.
    class Traversal {
        //  PROPERTIES  //

        /// @brief The associated traverse.
        Entry m_current = {};

        /// @brief Directory descriptor.
        ::DIR* m_descriptor = nullptr;

        /// @brief The prefix directory value.
        std::string m_prefix = Detail::getcwd();

       public:
        //  CONSTRUCTORS  //

        /**
         * @brief Constructs a UNIX directory traverser.
         * @param prefix                The prefix directory.
         */
        constexpr Traversal(const std::string& prefix = Detail::getcwd()) :
            m_descriptor(::opendir(prefix.c_str())), m_prefix(prefix) {}

        /// @brief Ensures we close a directory when necessary.
        constexpr ~Traversal() { m_release(); }

        //  PUBLIC METHODS  //

        /// @brief Denotes if currently done.
        inline constexpr bool done() const noexcept { return m_descriptor == nullptr; }

        /// @brief Gets the current iterator value.
        inline constexpr const Entry& current() const noexcept { return m_current; }

        /// @brief Advances the state of the traverser.
        inline constexpr const Entry& advance() {
            // stop whenever the descriptor is invalid
            if (m_descriptor == nullptr) return m_current;

            // attempt reading descriptors whilst we possibly can
            do { m_current = m_classify(::readdir(m_descriptor)); } while (m_ignored());

            // remove the descriptor when complete
            if (m_current.path == "") m_release();

            // return the resulting current value
            return m_current;
        }

       private:
        //  PRIVATE METHODS  //

        /// @brief Handles releasing the traversal descriptor.
        inline constexpr void m_release() {
            if (m_descriptor == nullptr) return;  // completed
            ::closedir(m_descriptor), m_descriptor = nullptr;
        }

        /**
         * @brief Checks for ignorable paths.
         * @param name                  Name of path.
         */
        inline constexpr bool m_ignored() const noexcept { return m_ignored(m_current.path); }
        inline constexpr bool m_ignored(const std::string_view& name) const noexcept {
            return name == "." || name == "..";
        }

        /**
         * @brief Handles classifying entries.
         * @param entry                 Entry to classify.
         */
        inline constexpr Entry m_classify(struct dirent* entry) const noexcept {
            // fail when necessary to do so
            if (entry == nullptr) return {};

            std::string suffix = entry->d_name;  // prepare the path details
            auto path = m_ignored(suffix) ? suffix : Detail::join(m_prefix, suffix);

            // and resolve the outgoing path to be used now
            return { path, m_archetype(entry) };
        }

        /**
         * @brief Gets the associated archetype.
         * @param entry                 Entry to classify.
         */
        inline constexpr Archetype m_archetype(struct dirent* entry) const noexcept {
            switch (entry->d_type) {
                case DT_REG: return Archetype::REGULAR;
                case DT_LNK: return Archetype::SYMLINK;
                case DT_DIR: return Archetype::DIRECTORY;
                default: return Archetype::INVALID;
            }
        }
    };

}  // namespace Aster::Detail

#endif
#endif
