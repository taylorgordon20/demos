#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace bourbon {

// Exception thrown when an any_cast is made with incompatible types.
class AnyCastException : public std::bad_cast {
 public:
  virtual const char* what() const noexcept {
    return "Contained type could not be cast to requested type.";
  }
};

// Provides a template-free container class that can own a value of any type.
// Instances of this class can be used inside collections in a typesafe way,
// while having distinct underlying types stored in the collection.
class Any {
  // An template-free base class used to polymorphically refer to the container
  // of an Any instance's value.
  class ContainerBase {
   public:
    virtual ~ContainerBase() {}
    virtual ContainerBase* clone() const = 0;
    virtual const std::type_info& type() const = 0;
  };

  // The type-specific holder of an Any instance's value.
  template <typename ValueType>
  class Container : public ContainerBase {
   public:
    Container(const ValueType& v) : value(v) {}
    Container(ValueType&& v) : value(std::forward<ValueType>(v)) {}
    ContainerBase* clone() const override {
      return new Container<ValueType>(value);
    }
    const std::type_info& type() const override {
      return typeid(ValueType);
    }

    ValueType value;
  };

 public:
  // Default constructor
  Any() {}

  // Copy constructor
  Any(const Any& other)
      : container_(other.container_ ? other.container_->clone() : nullptr) {}

  // Move constructor
  Any(Any&& other) noexcept {
    container_.swap(other.container_);
  }

  // Container value constructor
  template <typename ValueType> Any(const ValueType& value)
      : container_(new Container<std::decay_t<const ValueType>>(value)) {
  }

  // Perfect forwarding container value constructor
  template <typename ValueType,
            typename = std::enable_if_t<!std::is_same<Any&, ValueType>::value &&
                                        !std::is_const<ValueType>::value>>
  Any(ValueType&& value) : container_(
      new Container<std::decay_t<ValueType>>(std::forward<ValueType>(value))) {
  }

  // Copy assignment operator
  Any& operator=(const Any& other) {
    container_.reset(other.container_ ? other.container_->clone() : nullptr);
    return *this;
  }

  // Move assignment operator
  Any& operator=(Any&& other) noexcept {
    container_.swap(other.container_);
    other.container_.reset(nullptr);
    return *this;
  }

  // Perfect forwarding container value assignment operator
  template <typename ValueType> Any& operator=(ValueType&& value) {
    container_.reset(new Container<std::decay_t<ValueType>>(
        std::forward<ValueType>(value)));
    return *this;
  }
  
  // Queries against the Any type
  bool empty() { return !container_; }
  void clear() { container_.reset(nullptr); }
  const std::type_info& type() const {
    return container_ ? container_->type() : typeid(void);
  }

 private:
  std::unique_ptr<ContainerBase> container_;

  template <typename ValueType>
  friend ValueType any_cast(Any &any);
};

// Casts an Any lvalue reference to the provided type.
template <typename ValueType> ValueType any_cast(Any& any) {
  auto *container = any.container_.get();
  std::type_index cast_type(typeid(std::decay_t<ValueType>));
  if (container == nullptr || cast_type != std::type_index(container->type())) {
    throw AnyCastException();
  }

  // We need to cast the value to the requested ValueType (e.g. it might be an
  // lvalue or an rvalue reference type, which require explicit casting).
  // NOTE: This static_cast can cause unnecessary copying if ValueType is a
  // non-reference object type (e.g. string, vector will cause copying). We
  // could fix this by coercing the cast type into a reference type, but for now
  // we rely on return-value optimization to remove the unnecessary copy.
  return static_cast<ValueType>(
      static_cast<Any::Container<std::decay_t<ValueType>>*>(container)->value);
}

// Casts an Any rvalue reference to the provided type.
template <typename ValueType> ValueType any_cast(Any&& any) {
  // We need to make sure that ValueType is not a reference type since this Any
  // instance is an expiring object. A const reference type is allowed.
  static_assert(
      std::is_rvalue_reference<ValueType&&>::value ||
      std::is_const<std::remove_reference_t<ValueType>>::value,
      "Any rvalue references cannot be cast to non-const reference types.");
  return any_cast<ValueType>(any);
}

// Casts an Any pointer to a pointer of the provided type.
template <typename ValueType>
ValueType any_cast(Any* any) {
  static_assert(std::is_pointer<ValueType>::value,
                "A pointer to an Any type can only be cast to a pointer type.");
  return &any_cast<std::remove_pointer_t<ValueType>&>(*any);
}

} // namespace bourbon
