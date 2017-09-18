/** Common declarations. */

#ifndef SYNCER_BACKEND_COMMON_H_
#define SYNCER_BACKEND_COMMON_H_

#include <string>

namespace syncer {
  using namespace std;

  /** Buffer for textual or binary messages. */
  using Message = string;

  /** Buffer for textual or binary messages. */
  using ReplyHandler = function<void(bool, const Message&)>;

}

#endif // SYNCER_BACKEND_COMMON_H_
