#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>

#include "bifilter.hpp"
#include "timers.hpp"

#define EXPECT(cond)                          \
    if (!(cond)) {                            \
      throw std::runtime_error("Failed: " #cond); \
    }

namespace bourbon {

void testStrings() {
  // Define a test set of includes.
  std::vector<std::string> includes = {
    "taylor",
    "indre",
    "myles",
    "blake"
  };

  // Define a test set of excludes.
  std::vector<std::string> excludes = {
    "asad",
    "hussein",
    "barat",
    "kahleed"
  };
  
  Bifilter<std::string> bifilter(includes, excludes);
  EXPECT(bifilter.contains("taylor"));
  EXPECT(bifilter.contains("indre"));
  EXPECT(bifilter.contains("myles"));
  EXPECT(bifilter.contains("blake"));
  EXPECT(!bifilter.contains("asad"));
  EXPECT(!bifilter.contains("hussein"));
  EXPECT(!bifilter.contains("barat"));
  EXPECT(!bifilter.contains("kahleed"));
}

void testIntegers() {
  // Define a test set of includes.
  std::vector<std::string> includes(500);
  std::iota(includes.begin(), includes.end(), 0);

  // Define a test set of excludes.
  std::vector<std::string> excludes(500);
  std::iota(includes.begin(), includes.end(), 500);
  
  Bifilter<std::string> bifilter(includes, excludes);
  for (auto include : includes) {
    EXPECT(bifilter.contains(include));
  }
  for (auto exclude : excludes) {
    EXPECT(!bifilter.contains(exclude));
  }
}

} // namespace bourbon

int main() {
  using namespace bourbon;

  {
    Timer timer("Testing bifilter on strings");
    testStrings();
  }

  {
    Timer timer("Testing bifilter on set of integers");
    testIntegers();
  }

  std::cout << "Passed all tests!" << std::endl;

  for (int i = 0; i < 10; i += 1) {
    std::cout << "i -> " << std::hash<int>()(i) << std::endl;
  }
}
