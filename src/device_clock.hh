#ifndef CLOCK_HH_
#define CLOCK_HH_

#include <chrono>
#include <condition_variable>
#include <shared_mutex>
#include <stop_token>
#include <thread>

// This header provides a DeviceClock that abstracts away the Vulkan details of
// comparing CPU and GPU times.

namespace low_latency {

class DeviceContext;

// Satisfies the C++ Clock concept - std::chrono::time_point<DeviceClock> is a
// distinct type from std::chrono::steady_clock::time_point and this is useful
// because they should *never* be compared.
class DeviceClock final {
  public:
    using rep = std::chrono::nanoseconds::rep;
    using period = std::chrono::nanoseconds::period;
    using duration = std::chrono::nanoseconds;
    using time_point = std::chrono::time_point<DeviceClock>;
    static constexpr bool is_steady = true;

    const DeviceContext& device;

  private:
    void calibrate();
    void do_calibration(const std::stop_token stoken);

  private:
    std::condition_variable_any cv{};
    std::shared_mutex mutex{};
    std::uint64_t host_ns{};
    std::uint64_t error_bound{};
    std::uint64_t device_ticks{};

    // Every so often we recalibrate the clock on another thread.
    static constexpr auto CALIBRATION_PERIOD = std::chrono::seconds{1};
    std::jthread calibration_thread{};

  public:
    DeviceClock(const DeviceContext& device);
    DeviceClock(const DeviceClock&) = delete;
    DeviceClock(DeviceClock&&) = delete;
    DeviceClock operator=(const DeviceClock&) = delete;
    DeviceClock operator=(DeviceClock&&) = delete;
    ~DeviceClock();

  public:
    static time_point now();

  public:
    time_point ticks_to_time(const std::uint64_t& ticks);
};

} // namespace low_latency

#endif