#ifndef HCL_COMMON_HCL_LOGGING_H
#define HCL_COMMON_HCL_LOGGING_H
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif

#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define HCL_NOOP_MACRO do {} while (0)

//=============================================================================
#ifdef HCL_LOGGER_NO_LOG
//=============================================================================
#define HCL_LOGGER_INIT()              HCL_NOOP_MACRO
#define HCL_LOG_ERROR(...)   fprintf(stderr, __VA_ARGS__);
#define HCL_LOG_WARN(...)    HCL_NOOP_MACRO
#define HCL_LOG_INFO(...)    HCL_NOOP_MACRO
#define HCL_LOG_DEBUG(...)   HCL_NOOP_MACRO
#define HCL_LOG_TRACE(...)   HCL_NOOP_MACRO
#define HCL_LOG_STDOUT_REDIRECT(fpath) HCL_NOOP_MACRO
#define HCL_LOG_STDERR_REDIRECT(fpath) HCL_NOOP_MACRO
//=============================================================================
#else
//=============================================================================

#if defined(HCL_LOGGER_CPP_LOGGER) // CPP_LOGGER ---------------------------
  #include <cpp-logger/clogger.h>

  #define HCL_LOGGER_NAME "HCL"

  #ifdef HCL_LOGGER_LEVEL_DEBUG
    #define HCL_LOGGER_INIT() cpp_logger_clog_level(CPP_LOGGER_DEBUG, HCL_LOGGER_NAME);
  #elif defined(HCL_LOGGER_LEVEL_INFO)
    #define HCL_LOGGER_INIT() cpp_logger_clog_level(CPP_LOGGER_INFO, HCL_LOGGER_NAME);
  #elif defined(HCL_LOGGER_LEVEL_WARN)
    #define HCL_LOGGER_INIT() cpp_logger_clog_level(CPP_LOGGER_WARN, HCL_LOGGER_NAME);
  #else
    #define HCL_LOGGER_INIT() cpp_logger_clog_level(CPP_LOGGER_ERROR, HCL_LOGGER_NAME);
  #endif

  #define HCL_LOG_STDOUT_REDIRECT(fpath) freopen ((fpath), "a+", stdout);
  #define HCL_LOG_STDERR_REDIRECT(fpath) freopen ((fpath), "a+", stderr);


  #ifdef HCL_LOGGER_LEVEL_TRACE
    #define HCL_LOG_TRACE(...) cpp_logger_clog(CPP_LOGGER_TRACE, HCL_LOGGER_NAME, __VA_ARGS__);
  #else
    #define HCL_LOG_TRACE(...) HCL_NOOP_MACRO
  #endif

  #ifdef HCL_LOGGER_LEVEL_DEBUG
    #define HCL_LOG_DEBUG(...) cpp_logger_clog(CPP_LOGGER_DEBUG, HCL_LOGGER_NAME, __VA_ARGS__);
  #else
    #define HCL_LOG_DEBUG(...) HCL_NOOP_MACRO
  #endif

  #ifdef HCL_LOGGER_LEVEL_INFO
    #define HCL_LOG_INFO(...) cpp_logger_clog(CPP_LOGGER_INFO, HCL_LOGGER_NAME, __VA_ARGS__);
  #else
    #define HCL_LOG_INFO(...) HCL_NOOP_MACRO
  #endif

  #ifdef HCL_LOGGER_LEVEL_WARN
    #define HCL_LOG_WARN(...) cpp_logger_clog(CPP_LOGGER_WARN, HCL_LOGGER_NAME, __VA_ARGS__);
  #else
    #define HCL_LOG_WARN(...) HCL_NOOP_MACRO
  #endif

  #ifdef HCL_LOGGER_LEVEL_ERROR
    #define HCL_LOG_ERROR(...) cpp_logger_clog(CPP_LOGGER_ERROR, HCL_LOGGER_NAME, __VA_ARGS__);
  #else
    #define HCL_LOG_ERROR(...) HCL_NOOP_MACRO
  #endif
#else
#define HCL_LOGGER_INIT()              HCL_NOOP_MACRO
#define HCL_LOG_ERROR(...)   fprintf(stderr, __VA_ARGS__);
#define HCL_LOG_WARN(...)    HCL_NOOP_MACRO
#define HCL_LOG_INFO(...)    HCL_NOOP_MACRO
#define HCL_LOG_DEBUG(...)   HCL_NOOP_MACRO
#define HCL_LOG_TRACE(...)   HCL_NOOP_MACRO
#define HCL_LOG_STDOUT_REDIRECT(fpath) HCL_NOOP_MACRO
#define HCL_LOG_STDERR_REDIRECT(fpath) HCL_NOOP_MACRO
#endif // HCL_LOGGER_CPP_LOGGER -----------------------------------------------

//=============================================================================
#endif // HCL_LOGGER_NO_LOG
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif /* HCL_COMMON_HCL_LOGGING_H */