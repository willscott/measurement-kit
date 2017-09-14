// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_WORKER_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_WORKER_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include <chrono>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace mk {

class Worker {
  public:
    class State : public NonCopyable, public NonMovable {
      public:
        unsigned short active = 0;
        std::mutex mutex;
        unsigned short parallelism = 3;
        std::list<Callback<>> queue;
    };

    void call_in_thread(Callback<> &&func);

    unsigned short parallelism() const;

    void set_parallelism(unsigned short newval) const;

    unsigned short concurrency() const;

    void wait_empty_() const;

    static Worker &default_tasks_queue();

  private:
    SharedPtr<State> state{std::make_shared<State>()};
};

} // namespace mk
#endif
