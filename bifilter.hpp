#pragma once

#include <string>
#include <functional>
#include <unordered_set>
#include <set>
#include <vector>

namespace bourbon {

template <typename Key, typename Hash = std::hash<Key>>
class Bifilter {
 public:
  // Creates a bifilter that is guaranteed to test positive on the provided
  // includes set and test negative on the provided excludes set.
  template <typename Collection>
  Bifilter(const Collection& includes, const Collection& excludes);
  virtual ~Bifilter() = default;

  // Returns whether the given value is included in the filter.
  bool contains(const Key& value) const;

  // Serialization/deserialization methods.
  std::string serialize() const;
  static Bifilter<Key> deserialize(const std::string& encoding);

 protected:
  Bifilter() {}

 private:
  enum BucketState {
    INCLUDED = 0,
    EXCLUDED = 1,
    CONFLICT = 2
  };
  typedef std::vector<BucketState> Layer;
  std::vector<Layer> layers_;

  //std::unordered_set<size_t> test_;
  std::set<size_t> test_;
};

template <typename Key, typename Hash>
template <typename Collection>
Bifilter<Key, Hash>::Bifilter(
    const Collection& includes, const Collection& excludes) {
  for (const Key& value : includes) {
    test_.insert(Hash()(value));
  }
}

template <typename Key, typename Hash>
bool Bifilter<Key, Hash>::contains(
    const Key& value) const {
  return test_.count(Hash()(value));
}

template <typename Key, typename Hash>
std::string Bifilter<Key, Hash>::serialize() const {
  return "";
}

// static 
template <typename Key, typename Hash>
Bifilter<Key> Bifilter<Key, Hash>::deserialize(const std::string& encoding) {
  Bifilter<Key> ret;
  return ret;
}


} // namespace bourbon
