#include "path.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

auto get_executable_dir() -> std::filesystem::path {
    std::string path;

#if defined(_WIN32)

    std::string buffer(MAX_PATH, '\0');
    DWORD size = GetModuleFileNameA(nullptr, buffer.data(),
                                    static_cast<DWORD>(buffer.size()));

    if (size == 0) {
        return {};
    }

    buffer.resize(size);
    path = buffer;

#elif defined(__APPLE__)

    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::string buffer(size, '\0');

    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }

    path = buffer;

#elif defined(__linux__)

    std::string buffer(PATH_MAX, '\0');

    ssize_t count = readlink("/proc/self/exe", buffer.data(), buffer.size());

    if (count == -1) {
        return {};
    }

    buffer.resize(static_cast<size_t>(count));
    path = buffer;

#endif

    return std::filesystem::path(path).parent_path();
}
