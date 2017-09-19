/** ZeroMQ client implementation. */

#ifndef SYNCER_CLIENT_H_
#define SYNCER_CLIENT_H_

#include "backend.h"
#include "patch_op_handler.h"

namespace syncer {

using namespace std;
using namespace std::placeholders;

/**
 * Data-synchronizing client.
 * It creates both requester and subscriber at time of construction. Then the
 * requester retrieves a full data state from a remote Server. When the data is
 * changed the publisher will get notifications containing the corresponding
 * JSON patches (RFC 6902) and will apply them in order to make the local data
 * match the remote counterpart.
 *
 *
 */
template <typename T, typename Backend = SYNCER_DEFAULT_BACKEND> class Client {
 public:
  Client(const typename Backend::Config& req_conf,
         const typename Backend::Config& sub_conf,
         const PatchOpHandler<T>& handler)
      : req_(req_conf, bind(&Client::HandleReply, ref(*this), _1, _2))
      , sub_(sub_conf, bind(&Client::HandleNotification, ref(*this), _1))
      , handler_(handler) {

  }

 private:
  void HandleReply(bool success, const Message& reply) {

  }

  void HandleNotification(const Message& reply) {

  }

  typename Backend::Requester req_;
  typename Backend::Subscriber sub_;
  PatchOpHandler<T> handler_;
};

}

#endif // SYNCER_CLIENT_H_
