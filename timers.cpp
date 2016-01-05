#include <chrono>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <streambuf>
#include <vector>

#include "timers.hpp"

namespace bourbon {

using std::chrono::high_resolution_clock;
using std::chrono::duration;

namespace {

// Define an empty streambuf instance to consume output. 
class empty_streambuf : public std::streambuf {
 public:
  int overflow(int c) { return c; }
};

std::string& timer_prefix() {
  static std::string ret;
  return ret;
}

std::ostream& default_ostream() {
/*
#ifdef NDEBUG
  static empty_streambuf buf;
  static std::ostream ret(&buf);
  return ret;
#else
  return std::cout;
#endif
*/
  return std::cout;
}

} // namespace

Timer::Timer(const std::string& description)
    : Timer(description, default_ostream()) {
}

Timer::Timer(const std::string& description, std::ostream& output_stream)
    : output_stream_(output_stream) {
  timer_prefix() += "____";
  output_stream_ << timer_prefix() << description << "..." << std::endl;
  start_ = high_resolution_clock::now();
}

Timer::~Timer() {
  // Compute the duration as a double in seconds.
  auto end = high_resolution_clock::now();
  double millis = duration<double, std::milli>(end - start_).count();

  // Output the duration.
  output_stream_ << timer_prefix() << "Took " << millis << " ms" << std::endl;
  if (timer_prefix().size() >= 4) {
    timer_prefix().resize(timer_prefix().size() - 4);
  }
}

} // namespace bourbon
