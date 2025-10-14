# Aster - Fast C++ Globbing

Aster is an efficient, compact and robust cross-platform C++ [globbing](<https://en.wikipedia.org/wiki/Glob_(programming)>) library. This library provides a fast baseline globbing function and a walker for traversing the file-system.

## Installation

### CMake

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    aster
    GIT_REPOSITORY https://github.com/rroessler/cpp.aster.git
    GIT_TAG main  # can replace this with a specific version
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(aster)

target_link_libraries(${PROJECT_NAME} PRIVATE aster::aster)
```

And then the library will be accessible via:

```c++
#include <aster/aster.hpp>
```

### Headers

Since Aster is header-only, all files within `/include` can be used directly.

## Examples

### Glob Matching

```c++
auto glob = "some/**/path/**/n*[k-m]e?txt";
auto path = "some/small/or/large/path/to/a/needle.txt";
assert(Aster::Match::glob(glob, path)); // single match
```

### Glob Walking

```c++
auto options = Aster::Options( /** initial directory */ );
auto markdown = Aster::Walker("**/*.md"); // match ".md" files
for (const auto& entry : markdown.iterate()) { ... }
```

### Glob Options

```c++
struct Aster::Options {
    bool files = true;          // Allow matching files.
    bool hidden = false;        // Allow matching hidden entries.
    bool symlinks = false;      // Allow matching symlinks.
    bool directories = false;   // Allow matching directories.
    std::string cwd = "...";    // The initial working directory.
};
```

## License

This software is released under the terms of the MIT license.
