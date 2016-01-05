#pragma once

#include <chrono>
#include <ostream>
#include <string>

namespace bourbon {

// A simple tool for tracing the 
class Timer {
 public:
  Timer(const std::string& description);
  Timer(const std::string& description, std::ostream& output_stream);
  ~Timer();

 private:
  std::ostream& output_stream_;
  std::chrono::high_resolution_clock::time_point start_;
};

} // namespace bourbon
