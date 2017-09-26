/**
 * @file timer.h
 * @author Simon Prykhodko
 * @brief Simple timer implementation.
 */

#ifndef SYNCER_TIMER_H_
#define SYNCER_TIMER_H_

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace syncer {

/** @brief Simple timer for internal usage. */
class Timer {
 public:
  /** @brief Timer callback. */
  using Callback = std::function<void(void)>;

  /** @brief Constructor. */
  Timer() { }

  /**
   * @brief Constructor.
   * @param cb a callback to be called after delay.
   * @param delay a delay in milliseconds.
   */
  Timer(Callback cb, int delay) {
    Set(cb, delay);
  }

  /** @brief Destructor. */
  ~Timer() {
    Reset();
  }

  /**
   * @brief Set a delay.
   * @details Resets a previous delay if it was set before.
   * @param cb a callback to be called after delay.
   * @param delay a delay in milliseconds.
   */
  void Set(Callback cb, int delay) {
    Reset();

    exit_ = false;
    thr_ = std::thread(&Timer::Process, this, cb, delay);
  }

  /** @brief Reset a previous delay. */
  void Reset() {
    if (thr_.joinable()) {
      exit_ = true;
      thr_.join();
    }
  }

 private:
  static const int SLEEP_PERIOD = 100;

  void Process(Callback cb, int delay) {
    using namespace std;

    for (int period, elapsed = 0; !exit_; elapsed += period) {
      if (elapsed >= delay) {
        cb();
        return;
      }

      period = SLEEP_PERIOD;
      period = min(period, delay - elapsed);
      this_thread::sleep_for(chrono::milliseconds(period));
    }
  }

  std::atomic_bool exit_;
  std::thread thr_;
};

}

#endif // SYNCER_TIMER_H_
