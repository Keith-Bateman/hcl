#ifndef HCL_COMMON_HCL_LOGGING_H
#define HCL_COMMON_HCL_LOGGING_H
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <string>
#ifdef __cplusplus
extern "C" {
#endif
#define VA_ARGS(...) , ##__VA_ARGS__

std::string get_time() {
  auto hcl_ts_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count() %
                       1000;
  auto hcl_ts_t = std::time(0);
  auto now = std::localtime(&hcl_ts_t);
  char hcl_ts_time_str[sizeof "9999-12-31 29:59:59.9999"];
  sprintf(hcl_ts_time_str, "%04d-%02d-%02d %02d:%02d:%02d.%ld",
          now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour,
          now->tm_min, now->tm_sec, hcl_ts_millis);
  return hcl_ts_time_str;
}

// #define HCL_NOOP_MACRO do {} while (0)

//=============================================================================
#ifdef HCL_LOGGER_NO_LOG
//=============================================================================
#define HCL_LOGGER_INIT() HCL_NOOP_MACRO
#define HCL_LOG_ERROR(format, ...) fprintf(stderr, __VA_ARGS__);
#define HCL_LOG_WARN(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_INFO(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_DEBUG(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_TRACE() HCL_NOOP_MACRO
#define HCL_LOG_TRACE_FORMAT(...) HCL_NOOP_MACRO
#define HCL_LOG_STDOUT_REDIRECT(fpath) HCL_NOOP_MACRO
#define HCL_LOG_STDERR_REDIRECT(fpath) HCL_NOOP_MACRO
//=============================================================================
#else
//=============================================================================

#if defined(HCL_LOGGER_CPP_LOGGER)  // CPP_LOGGER
                                    // ---------------------------
#include <cpp-logger/clogger.h>

#define HCL_LOG_STDOUT_REDIRECT(fpath) freopen((fpath), "a+", stdout);
#define HCL_LOG_STDERR_REDIRECT(fpath) freopen((fpath), "a+", stderr);
#define HCL_LOGGER_NAME "HCL"

#define HCL_INTERNAL_TRACE(file, line, function, name, logger_level)         \
  cpp_logger_clog(logger_level, name, "[%s] %s [%s:%d]", get_time().c_str(), \
                  function, file, line);

#define HCL_INTERNAL_TRACE_FORMAT(file, line, function, name, logger_level, \
                                  format, ...)                              \
  cpp_logger_clog(logger_level, name, "[%s] %s " format " [%s:%d]",         \
                  get_time().c_str(), function, __VA_ARGS__, file, line);

#ifdef HCL_LOGGER_LEVEL_TRACE
#define HCL_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_TRACE, HCL_LOGGER_NAME);
#elif defined(HCL_LOGGER_LEVEL_DEBUG)
#define HCL_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_DEBUG, HCL_LOGGER_NAME);
#elif defined(HCL_LOGGER_LEVEL_INFO)
#define HCL_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_INFO, HCL_LOGGER_NAME);
#elif defined(HCL_LOGGER_LEVEL_WARN)
#define HCL_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_WARN, HCL_LOGGER_NAME);
#else
#define HCL_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_ERROR, HCL_LOGGER_NAME);
#endif

#ifdef HCL_LOGGER_LEVEL_TRACE
#define HCL_LOG_TRACE()                                                 \
  HCL_INTERNAL_TRACE(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                     CPP_LOGGER_TRACE);
#define HCL_LOG_TRACE_FORMAT(format, ...)                                      \
  HCL_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                            CPP_LOGGER_TRACE, format, __VA_ARGS__);
#else
#define HCL_LOG_TRACE(...) HCL_NOOP_MACRO
#define HCL_LOG_TRACE_FORMAT(...) HCL_NOOP_MACRO
#endif

#ifdef HCL_LOGGER_LEVEL_DEBUG
#define HCL_LOG_DEBUG(format, ...)                                             \
  HCL_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                            CPP_LOGGER_DEBUG, format, __VA_ARGS__);
#else
#define HCL_LOG_DEBUG(format, ...) HCL_NOOP_MACRO
#endif

#ifdef HCL_LOGGER_LEVEL_INFO
#define HCL_LOG_INFO(format, ...)                                              \
  HCL_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                            CPP_LOGGER_INFO, format, __VA_ARGS__);
#else
#define HCL_LOG_INFO(format, ...) HCL_NOOP_MACRO
#endif

#ifdef HCL_LOGGER_LEVEL_WARN
#define HCL_LOG_WARN(format, ...)                                              \
  HCL_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                            CPP_LOGGER_WARN, format, __VA_ARGS__);
#else
#define HCL_LOG_WARN(format, ...) HCL_NOOP_MACRO
#endif

#ifdef HCL_LOGGER_LEVEL_ERROR
#define HCL_LOG_ERROR(format, ...)                                             \
  HCL_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__, HCL_LOGGER_NAME, \
                            CPP_LOGGER_ERROR, format, __VA_ARGS__);
#else
#define HCL_LOG_ERROR(format, ...) HCL_NOOP_MACRO
#endif
#else
#define HCL_LOGGER_INIT() HCL_NOOP_MACRO
#define HCL_LOG_ERROR(format, ...) fprintf(stderr, __VA_ARGS__);
#define HCL_LOG_WARN(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_INFO(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_DEBUG(format, ...) HCL_NOOP_MACRO
#define HCL_LOG_TRACE() HCL_NOOP_MACRO
#define HCL_LOG_TRACE_FORMAT(...) HCL_NOOP_MACRO
#define HCL_LOG_STDOUT_REDIRECT(fpath) HCL_NOOP_MACRO
#define HCL_LOG_STDERR_REDIRECT(fpath) HCL_NOOP_MACRO
#endif  // HCL_LOGGER_CPP_LOGGER
        // -----------------------------------------------

//=============================================================================
#endif  // HCL_LOGGER_NO_LOG
        //=============================================================================

#ifdef __cplusplus
}
#endif

#endif /* HCL_COMMON_HCL_LOGGING_H */