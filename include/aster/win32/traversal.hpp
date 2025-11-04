#ifndef _ASTER_TRAVERSAL_WIN32_HPP
#define _ASTER_TRAVERSAL_WIN32_HPP

/// Aster Includes
#include "aster/entry.hpp"

#ifdef _ASTER_PLATFORM_WIN32

namespace Aster::Detail {

    /// @brief Directory Traversal Implementation.
    class Traversal {
        //  PROPERTIES  //

        /// @brief The associated traverse.
        Entry m_current = {};

        /// @brief Directory descriptor.
        WIN32_FIND_DATAA m_data = {};

        /// @brief File-handle reference.
        HANDLE m_stream = INVALID_HANDLE_VALUE;

        /// @brief The prefix directory value.
        std::string m_prefix = Detail::getcwd();

       public:
        //  CONSTRUCTORS  //

        /**
         * @brief Constructs a Win32 directory traverser.
         * @param prefix                Prefix directory.
         */
        constexpr explicit Traversal(const std::string& prefix = Detail::getcwd()) : m_prefix(prefix) {
            m_stream = ::FindFirstFileA((m_prefix + "\\*").c_str(), &m_data);
        }

        /// @brief Ensures we close a directory when necessary.
        constexpr ~Traversal() { m_release(); }

        //  PUBLIC METHODS  //

        /// @brief Denotes if currently done.
        inline constexpr bool done() const noexcept { return m_stream == INVALID_HANDLE_VALUE; }

        /// @brief Gets the current iterator value.
        inline constexpr const Entry& current() const noexcept { return m_current; }

        /// @brief Advances the state of the traverser.
        inline constexpr const Entry& advance() {
            // stop if the file-handle is invalid
            if (m_stream == INVALID_HANDLE_VALUE) return m_current;

            // attempt checking for the next stream value now
            do { m_current = m_classify(::FindNextFileA(m_stream, &m_data)); } while (m_ignored());

            // remove the descriptor when complete
            if (m_current.path == "") m_release();

            // return the resulting current value
            return m_current;
        }

       private:
        //  PRIVATE METHODS  //

        /// @brief Handles releasing the stream.
        inline constexpr void m_release() {
            if (m_stream == INVALID_HANDLE_VALUE) return;  // ignore
            ::FindClose(m_stream), m_stream = INVALID_HANDLE_VALUE;
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
         * @param success               Found success.
         */
        inline constexpr Entry m_classify(bool found) const noexcept {
            // failed to find valid match so we return as empty
            if (!found) return {};

            std::string suffix = m_data.cFileName;  // prepare the path details
            auto path = m_ignored(suffix) ? suffix : Detail::join(m_prefix, suffix);

            // and resolve the outgoing path to be used now
            return { path, m_archetype() };
        }

        /// @brief Gets the current associated archetype.
        inline constexpr Archetype m_archetype() const noexcept {
            auto symlink = m_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK;  // check for symlink tags
            if (m_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT && symlink) return Archetype::SYMLINK;
            if (m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return Archetype::DIRECTORY;
            return Archetype::REGULAR;  // otherwise was a regular file
        }
    };

}  // namespace Aster::Detail

#endif
#endif
