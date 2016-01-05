#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "any.hpp"

class Registry;
class Scope;

template <typename ValueType>
class RegistryKey {
 public:
  typedef ValueType type;
  ValueType get(const Registry& registry) {
    throw std::runtime_error("Requested registry without a default value");
  }
};

class Registry {
 public:
  template <typename KeyType> void put(typename KeyType::type&& value);
  template <typename KeyType> void drop();
  template <typename KeyType> void prepare() const;
  template <typename KeyType> typename KeyType::type& get() const;

  template <typename ValueType> void putDefault(ValueType&& value);
  template <typename ValueType> ValueType& getDefault() const;
  template <typename ValueType> void dropDefault() const;

 private:
  std::vector<Scope*> scopes_;
  mutable std::unordered_map<std::type_index, bourbon::Any> memo_;

  friend class Scope;
};

class Scope {
 public:
  Scope(Registry* registry);
  ~Scope();
 
 private:
  void add_key(std::type_index key);

  Registry* registry_;
  std::vector<std::type_index> keys_;

  friend class Registry;
};

template <typename KeyType>
void Registry::put(typename KeyType::type&& value) {
  using ValueType = typename KeyType::type;
  static_assert(std::is_base_of<RegistryKey<ValueType>, KeyType>::value,
                "Attempt to put a non-RegistryKey type in the Registry");
  const auto& key = std::type_index(typeid(KeyType));
  memo_[key] = std::forward<ValueType>(value);
}

template <typename KeyType>
void Registry::prepare() const {
  using ValueType = typename KeyType::type;
  static_assert(std::is_base_of<RegistryKey<ValueType>, KeyType>::value,
                "Attempt to prepare a non-RegistryKey type in the Registry");
  const auto& key = std::type_index(typeid(KeyType));
  if (!memo_.count(key)) {
    memo_[key] = KeyType().get(*this);
    if (scopes_.size()) {
      scopes_.back()->add_key(std::move(key));
    }
  }
}

template <typename KeyType>
void Registry::drop() {
  using ValueType = typename KeyType::type;
  static_assert(std::is_base_of<RegistryKey<ValueType>, KeyType>::value,
                "Attempt to drop a non-RegistryKey type in the Registry");
  const auto& key = std::type_index(typeid(KeyType));
  memo_.erase(key);
}

template <typename KeyType>
typename KeyType::type& Registry::get() const {
  prepare<KeyType>();
  const auto& key = std::type_index(typeid(KeyType));
  return bourbon::any_cast<typename KeyType::type&>(memo_[key]);
}

template <typename ValueType>
void Registry::putDefault(ValueType&& value) {
  put<RegistryKey<ValueType>>(std::forward<ValueType>(value));
}

template <typename ValueType>
ValueType& Registry::getDefault() const {
  return get<RegistryKey<ValueType>>();
}

template <typename ValueType>
void Registry::dropDefault() const {
  drop<RegistryKey<ValueType>>();
}

Scope::Scope(Registry* registry) : registry_(registry) {
  registry_->scopes_.push_back(this);
}

Scope::~Scope() {
  for (const auto& key : keys_) {
    registry_->memo_.erase(key);
  }
}

void Scope::add_key(std::type_index key) {
  keys_.emplace_back(std::move(key));
}


// A node with a default provider and no deps
class Foo : public RegistryKey<std::string> {
 public:
  std::string get(const Registry& registry) {
    return "Foo";
  }
};

// A node without a default provider
class Bar : public RegistryKey<std::string> {};

// A node with a default provider and deps
class FooBar : public RegistryKey<std::string> {
 public:
  std::string get(const Registry& registry) {
    const auto& foo = registry.get<Foo>();
    const auto& bar = registry.get<Bar>();
    return foo + ":" + bar;
  }
};

// A node that provides different values on each call
class Jazz : public RegistryKey<int> {
 public:
  int get(const Registry& registry) {
    static int ret = 0;
    return ret++;
  }
};

struct MyStruct {
  int x;
  float y;
};

int main() {
  Registry registry;
  registry.put<Bar>("bar");
  registry.putDefault<MyStruct>({13, 4.05f});
  std::cout << "FooBar=" << registry.get<FooBar>() << std::endl;
  std::cout << "MyStruct.x=" << registry.getDefault<MyStruct>().x << std::endl
            << "MyStruct.y=" << registry.getDefault<MyStruct>().y << std::endl;

  {
    Scope scope(&registry);
    int a = registry.get<Jazz>();
    int b = registry.get<Jazz>();
    std::cout << "a=" << a << ", b=" << b << std::endl;
  }

  {
    Scope scope(&registry);
    int a = registry.get<Jazz>();
    int b = registry.get<Jazz>();
    std::cout << "a=" << a << ", b=" << b << std::endl;
  }
}
