/**
 * @file common.h
 * @author Simon Prykhodko
 * @brief Common declarations.
 */

#ifndef SYNCER_COMMON_H_
#define SYNCER_COMMON_H_

#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#ifndef SYNCER_LOG
  #include <iostream>
  /**
   * @brief Log a message.
   * @details To customize logging this macro should be redefined by user.
   */
  #define SYNCER_LOG(msg) { std::cerr << "syncer: " << msg << std::endl; }
#endif

/** @brief Log a formatted message. */
#define SYNCER_LOG_FMT(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__)); }

/** @brief Log a formatted message with error description. */
#define SYNCER_LOG_ERROR(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__) + ": " + strerror(errno)); }

/** @brief Log a formatted message with exception message. */
#define SYNCER_LOG_EXCEPTION(e, fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__) + ": " + e.what()); }

#if not defined (SYNCER_NOEXCEPTION) \
      && (defined (__cpp_exceptions) || defined (__EXCEPTIONS) \
            || defined (_CPPUNWIND))
  #define SYNCER_TRY try
  #define SYNCER_CATCH_LOG(fmt, ...) \
    catch (const std::exception& e) \
      { SYNCER_LOG_EXCEPTION(e, fmt, ##__VA_ARGS__); }
#else
  /** @brief Safe `try`-wrapper. */
  #define SYNCER_TRY if (true)
  /** @brief Catch and log exception. */
  #define SYNCER_CATCH_LOG(fmt, ...)
#endif

/** @brief Syncer stuff. */
namespace syncer {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"

/** @brief Format standard strings as sprintf() does. */
template <typename ... Args>
std::string string_format(const std::string& fmt, Args ... args) {
  using namespace std;
  size_t size = snprintf(nullptr, 0, fmt.c_str(), args ...) + 1;
  unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, fmt.c_str(), args ...);
  return string(buf.get(), buf.get() + size - 1);
}

#pragma GCC diagnostic pop

/** @brief Socket type. */
enum class SocketType { PUBLISHER, REPLIER, REQUESTER, SUBSCRIBER };

}

#endif // SYNCER_COMMON_H_
