#include "BuildInfo.h"

#include "git_version.h"

#define VERSION_STRING(name, a, b, c) (std::string(name) + " " + std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c))

using namespace Phi;

static std::string compilerVersionString() {
    #ifdef __clang__
    return VERSION_STRING("clang", __clang_major__, __clang_minor__, __clang_patchlevel__);
    #elif __GNUC__
    return VERSION_STRING("GCC", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #elif _MSC_VER
    return std::string("Microsoft Visual C++ ") + _MSC_VER;
    #else
    return "unknown compiler"
    #endif
}

std::string BuildInfo::versionString() {
    std::string dev = "";
    if (std::string(BuildInfo::GIT_TAG) != std::string(BuildInfo::GIT_VER_STRING)) {
        dev = "-dev";
    }
    return std::string(BuildInfo::GIT_TAG) + dev + " (" + BuildInfo::GIT_VER_STRING + " " + compilerVersionString() + ")";
}