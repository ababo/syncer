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
 * @brief Log a string part and further parts are to follow.
 * @details To customize logging this macro should be redefined by user.
 */
#define SYNCER_LOG(msg) { cerr << "syncer: " << endl; }

#endif // SYNCER_LOG

#define SYNCER_LOG_FMT(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__)); }

#define SYNCER_LOG_FMTE(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__) + ": " + strerror(errno)); }

/** @brief Syncer stuff. */
namespace syncer {

using namespace std;

/** @brief Format standard strings as sprintf() does. */
template<typename ... Args>
string string_format(const string& format, Args ... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return string(buf.get(), buf.get() + size - 1);
}

/** @brief Buffer for textual or binary messages. */
using Message = string;

/** @brief Maximal size of transmitted or received message. */
static const int MAX_MSG_SIZE = 1024 * 1024;

/** @brief Socket waiting timeout in milliseconds. */
static const int SOCKET_WAIT_TIMEOUT = 100;

/** @brief Socket type. */
enum class SocketType { PUBLISHER, REPLIER, REQUESTER, SUBSCRIBER };

}

#endif // SYNCER_COMMON_H_
