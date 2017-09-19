/** ZeroMQ socket configuration. */

#ifndef SYNCER_ZMQ_CONFIG_H_
#define SYNCER_ZMQ_CONFIG_H_

namespace syncer {
namespace zmq {

using namespace std;

/** ZeroMQ socket configuration. */
struct Config {
  /** 
   * Default constructor.
   * Part of the backend template API.
   */
  Config() {
    sndhwm = 0;
    rcvhwm = 0;
    io_threads = 1;
  }

  /** 
   * Connection string constructor.
   * Part of the backend template API.
   * @param conn_str a ZeroMQ endpoint.
   */
  Config(const char* conn_str)
      : Config() {
    this->endpoint = conn_str;
  }

  /** ZeroMQ endpoint. */
  string endpoint;

  /** ZeroMQ ZMQ_SNDHWM value. */
  int sndhwm;

  /** ZeroMQ ZMQ_RCVHWM value. */
  int rcvhwm;

  /** Size of ZeroMQ thread pool. */
  int io_threads;
};

}
}

#endif // SYNCER_ZMQ_CONFIG_H_
