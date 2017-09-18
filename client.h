#ifndef SYNCER_CLIENT_H_
#define SYNCER_CLIENT_H_

#include "backend.h"

namespace syncer {

template <typename T, typename Backend = SYNCER_DEFAULT_BACKEND> class Client {
 public:
  Client(const typename Backend::Config& req_conf,
         const typename Backend::Config& sub_conf)
      : req_(req_conf)
      , sub_(sub_conf) {

  }

 private:
  typename Backend::Requester req_;
  typename Backend::Subscriber sub_;
};

}

#endif // SYNCER_CLIENT_H_
