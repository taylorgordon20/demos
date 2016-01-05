#include <iostream>
#include <limits>
#include <random>

#include "timers.hpp"

struct DatStruct {
  int dat1[1024];
  int dat2[1024];
};

int main() {
  std::random_device seed;
  std::default_random_engine engine(seed());
  std::uniform_int_distribution<int> distro(
      std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

  DatStruct src;

  const size_t n = sizeof(DatStruct::dat1) / sizeof(int);
  for (int i = 0; i < n; i += 1) {
    src.dat1[i] = distro(engine);
    src.dat2[i] = distro(engine);
  }

  const size_t num_trials = 1000;
  std::vector<DatStruct> copies(num_trials);
  {
    bourbon::Timer timer("Timing memcpy");
    for (int i = 0; i < num_trials; i += 1) {
      memcpy(&copies[i].dat1, &src.dat1, n);
      memcpy(&copies[i].dat2, &src.dat2, n);
    }
  }
}
