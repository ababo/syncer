/**
 * @file common.h
 * @author Simon Prykhodko
 * @brief Common declarations.
 */

#ifndef SYNCER_COMMON_H_
#define SYNCER_COMMON_H_

#include <cstdio>
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

#endif // SYNCER_LOG

/** @brief Log a formatted message. */
#define SYNCER_LOG_FMT(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__)); }

/** @brief Log a formatted message with error description. */
#define SYNCER_LOG_FMTE(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__) + ": " + strerror(errno)); }

/** @brief Syncer stuff. */
namespace syncer {

/** @brief Format standard strings as sprintf() does. */
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
  using namespace std;
  size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
  unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args ...);
  return string(buf.get(), buf.get() + size - 1);
}

/** @brief Socket type. */
enum class SocketType { PUBLISHER, REPLIER, REQUESTER, SUBSCRIBER };

}

#endif // SYNCER_COMMON_H_
