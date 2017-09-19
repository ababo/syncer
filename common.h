/** Common declarations (also used by backends). */

#ifndef SYNCER_COMMON_H_
#define SYNCER_COMMON_H_

#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#ifndef SYNCER_LOG

#include <iostream>

/**
 * Log a string part and further parts are to follow.
 * To customize logging this macro should be redefined by user.
 */
#define SYNCER_LOG(msg) { cerr << "syncer: " << endl; }

#endif // SYNCER_LOG

#define SYNCER_LOG_FMT(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__)); }

#define SYNCER_LOG_FMTE(fmt, ...) \
{ SYNCER_LOG(string_format(fmt, ##__VA_ARGS__) + ": " + strerror(errno)); }

namespace syncer {

using namespace std;

/** Format standard strings as sprintf() does. */
template<typename ... Args>
string string_format(const string& format, Args ... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return string(buf.get(), buf.get() + size - 1);
}

/** Buffer for textual or binary messages. */
using Message = string;

/** Callback to handle notifications by Subscriber. */
using NotificationCallback = function<void(const Message&)>;

/** Callback to handle requests by Replier. */
using RequestCallback = function<Message(const Message&)>;

/** Callback to handle replies by Requester. */
using ReplyCallback = function<void(bool, const Message&)>;

}

#endif // SYNCER_COMMON_H_
