#ifndef _ASTER_ENTRY_HPP
#define _ASTER_ENTRY_HPP

/// Aster Includes
#include "aster/detail.hpp"

namespace Aster {

    /// @brief Available Entry Types.
    enum class Archetype : uint8_t { INVALID, REGULAR, DIRECTORY, SYMLINK };

    /// @brief Iterator Entry Result.
    struct Entry {
        //  PROPERTIES  //

        /// @brief Bound file-path.
        std::string path = "";

        /// @brief Denotes an unknown entry.
        Archetype type = Archetype::INVALID;

        //  CONSTRUCTORS  //

        /// @brief Constructs a defaulted entry.
        constexpr Entry() = default;

        /**
         * @brief Constructs an explicit entry.
         * @param path              Entry path.
         * @param type              Entry type.
         */
        constexpr Entry(const std::string& path, Archetype type) : path(path), type(type) {}
    };

}  // namespace Aster

#endif
