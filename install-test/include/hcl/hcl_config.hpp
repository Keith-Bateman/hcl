#ifndef HCL_CONFIG_HPP
#define HCL_CONFIG_HPP

/* Version string for HCL */
#define HCL_PACKAGE_VERSION @HCL_PACKAGE_VERSION @
#define HCL_GIT_VERSION @HCL_GIT_VERSION @

/* Compiler used */
#define CMAKE_BUILD_TYPE "Debug"

#define CMAKE_C_COMPILER "/usr/tce/bin/gcc"
#define CMAKE_C_FLAGS " -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-deprecated-declarations"
#define CMAKE_C_FLAGS_DEBUG "-g"
#define CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG"
#define CMAKE_C_FLAGS_RELEASE " -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-deprecated-declarations_RELEASE"

#define CMAKE_CXX_COMPILER "/usr/tce/bin/g++"
#define CMAKE_CXX_FLAGS " -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wnon-virtual-dtor -Wno-deprecated-declarations"
#define CMAKE_CXX_FLAGS_DEBUG "-g"
#define CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG"
#define CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG"

/* #undef CMAKE_C_SHARED_LIBRARY_FLAGS */
/* #undef CMAKE_CXX_SHARED_LIBRARY_FLAGS */

/* Macro flags */
/* #undef HCL_GNU_LINUX */
#define HCL_COMMUNICATION_ENABLE_THALLIUM 1
/* #undef HCL_COMMUNICATION_PROTOCOL_ENABLE_OFI */
#define HCL_COMMUNICATION_PROTOCOL_ENABLE_UCX 1
#define HCL_HAS_STD_FILESYSTEM 1
#define HCL_HAS_STD_FSTREAM_FD 1
// Profiler
#define HCL_PROFILER_DLIO_PROFILER 1
/* #undef HCL_PROFILER_NONE */
// Logger
#define HCL_LOGGER_CPP_LOGGER 1
// Logger level
/* #undef HCL_LOGGER_NO_LOG */
#define HCL_LOGGER_LEVEL_ERROR 1
#define HCL_LOGGER_LEVEL_WARN 1
#define HCL_LOGGER_LEVEL_INFO 1
/* #undef HCL_LOGGER_LEVEL_DEBUG */
/* #undef HCL_LOGGER_LEVEL_TRACE */

//==========================
// Common macro definitions
//==========================

#define HCL_PATH_DELIM "/"

// #define HCL_NOOP_MACRO do {} while (0)
#define HCL_NOOP_MACRO

// Detect VAR_OPT
// https://stackoverflow.com/questions/48045470/portably-detect-va-opt-support
#if __cplusplus <= 201703 && defined __GNUC__ && !defined __clang__ && \
    !defined __EDG__
#define VA_OPT_SUPPORTED false
#else
#define PP_THIRD_ARG(a, b, c, ...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(, ), true, false, )
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)
#endif

#if !defined(HCL_HASH_SEED) || (HCL_HASH_SEED <= 0)
#define HCL_SEED 104723u
#endif

#endif /* HCL_CONFIG_H */
