#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "any.hpp"

class Processor;

class Node {
 public:
  virtual ~Node() = default;
  const std::type_info& type() = 0;
};

template <typename ValueType>
class Node<ValueType> {
 public:
  virtual ~Node() = default;
  virtual ValueType get(const Processor& processor) = 0;
  const std::type_info& type() const override {
    return typeid(ValueType);
  }
};

class NodeRegistry {
 public:
  template <typename NodeType,
            typename = std::enable_if_t<std::is_base_of<Node, NodeType>>::value>
  NodeType& get() const {
    auto key = std::type_index(std::typeid(NodeType));
    if (key == 
  }

 protected:
  mutable std::unordered_map<std::type_index, Node> nodes_;
};

class Processor {
 public:
  template <typename T> const T& get() const;
  template <typename T> std::unique_ptr<T> release();

 protected:
  typedef std::type_index NodeKey;
  template <typename T> NodeKey memoize() const;

 private:
  mutable std::unordered_map<NodeKey, bourbon::Any> memo_;
};

template <typename ValueType>
Node<ValueType> NodeRegistry::get() const {
  auto key = 
}

template <typename NodeType,
          typename = std::enable_if_t<std::is_base_of<Node, NodeType>>::value>
const T& Processor::get() const {
  auto key = memoize<NodeType>();
  return memo_[key];
}

template <typename NodeType,
          typename = std::enable_if_t<std::is_base_of<Node, NodeType>>::value>
std::unique_ptr<T> Processor::release() {
  auto key = memoize<NodeType>();
  auto ret = std::make_unique<NodeType>(
      std::move(bourbon::any_cast<NodeType>(memo_[key])));
  memo_.erase(key);
  return ret;
}

template <typename NodeType,
          typename = std::enable_if_t<std::is_base_of<Node, NodeType>>::value>
Processor::NodeKey Processor::memoize() const {
  NodeType node;
  auto key = std::type_index(node.type());
  if (!memo_.count(key)) {
    memo_[key] = std::move(node.get());
  }
  return key;
}

struct ViewerSrc {
  int64_t id;
  std::string locale;
  std::string gender;
};

struct MediaSrc {
  int64_t id;
  int num_likes;
};

struct FeatureMap {
  std::unordered_map<std::string, int64_t> ints;
  std::unordered_map<std::string, float> floats;
};

class ViewerFeatures : public Node<FeatureMap> {
 public:
  FeatureMap get(const Processor& processor) {
    ViewerSrc viewer_src = processor.get<ViewerSrc>();

    FeatureMap fm;
    fm.ints["viewer_id"] = viewer_src.id;
    fm.ints["viewer_locale"] = viewer_src.locale;
    fm.ints["viewer_gender"] = viewer_src.gender;
    return fm;
  }
};

class MediaFeatures : public Node<FeatureMap> {
 public:
  FeatureMap get(const Processor& processor) {
    MediaSrc media_src = processor.get<MediaSrc>();

    FeatureMap fm;
    fm.ints["media_id"] = media_src.id;
    fm.ints["media_num_likes"] = media_src.num_likes;
    return fm;
  }
};

class FeedFeatures : public Node<FeatureMap> {
 public:
  FeatureMap get(const Processor& processor) {
    FeatureMap fm;

    // Merge in the user features.
    FeatureMap uf = processor.get<UserFeatures>();
    fm.ints.insert(uf.ints.begin(), uf.ints.end());
    fm.floats.insert(uf.floats.begin(), uf.floats.end());

    // Merge in the media features.
    FeatureMap mf = processor.get<MediaFeatures>();
    fm.ints.insert(mf.ints.begin(), mf.ints.end());
    fm.floats.insert(mf.floats.begin(), mf.floats.end());

    return fm;
  }
};

int main() {
  Foo foo{1, 2, 2.1f};
  Bar bar{2.5f, 3.3f, 7};

  Processor processor;
  processor.put(std::move(foo));
  processor.put(std::move(bar));
  auto fm = processor.release<FeedFeatures>();
}
