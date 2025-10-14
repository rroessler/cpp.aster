#ifndef _ASTER_DETAIL_HPP
#define _ASTER_DETAIL_HPP

/// C++ Includes
#include <cstdlib>
#include <string>
#include <string_view>

#ifdef _WIN32
#define _ASTER_PLATFORM_WIN32
#define _ASTER_PLATFORM_GETCWD ::_getcwd

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

/// OS Includes
#include <direct.h>
#include <windows.h>
#else
#define _ASTER_PLATFORM_UNIX
#define _ASTER_PLATFORM_GETCWD ::getcwd

/// OS Includes
#include <unistd.h>
#endif

namespace Aster::Detail {

    /// @brief Describes a wildcard/globstar sequence.
    static inline constexpr std::string_view wildcard() { return "*"; }
    static inline constexpr std::string_view globstar() { return "**"; }

    /// @brief Describes an extension wildcard sequence.
    static inline constexpr std::string_view extends() { return "*."; }

    /// @brief Describes all special characters.
    static inline constexpr std::string_view special() { return "*[{?"; }
    static inline constexpr bool special(const std::string_view& view) {
        return view.find_first_of(special()) != std::string_view::npos;
    }

    /// @brief Gets the platforms separator whitelist.
    static inline constexpr std::string_view separator() {
#ifdef _ASTER_PLATFORM_WIN32
        return "/\\";
#else
        return "/";
#endif
    }

    /**
     * @brief Checks if a character is a separator.
     * @param ch                Character to valid.
     */
    static inline constexpr bool separator(char ch) {
#ifdef _ASTER_PLATFORM_WIN32
        return separator().contains(ch);
#else
        return ch == '/';
#endif
    }

    /**
     * @brief Handles join path segments together.
     * @param prefix            Prefix segment.
     * @param suffix            Suffix segment.
     */
    static inline constexpr std::string join(const std::string& prefix, const std::string& suffix) {
#ifdef _ASTER_PLATFORM_WIN32
        return prefix + "\\" + suffix;
#else
        return prefix + '/' + suffix;
#endif
    }

    /// @brief Allows getting the current-working directory.
    static inline constexpr std::string getcwd() {
        char* buffer = _ASTER_PLATFORM_GETCWD(nullptr, 0);
        return std::string(buffer), std::free(buffer), buffer;
    }

}  // namespace Aster::Detail

#endif
