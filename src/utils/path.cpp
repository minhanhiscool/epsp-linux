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
    std::wstring wpath(MAX_PATH, '\0');
    DWORD size = GetModuleFileNameW(nullptr, wpath.data(), wpath.size());
    wpath.resize(size);
    path = std::string(wpath.begin(), wpath.end());

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    _NSGetExecutablePath(buffer.data(), &size);
    path = buffer;

#elif defined(__linux__)
    std::string buffer(1024, '\0');
    ssize_t count = readlink("/proc/self/exe", buffer.data(), buffer.size());
    buffer.resize(count);
    path = buffer;
#endif

    return std::filesystem::path(path).parent_path();
}
