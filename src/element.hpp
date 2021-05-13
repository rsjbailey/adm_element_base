#pragma once
#include <tuple>
#include <optional>
#include <type_traits>
#include <iostream>
#include <cassert>

template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template<typename T>
T getDefault() {
  return T{};
}

template<typename T>
class DefaultedParam {
 public:
  DefaultedParam() : value{getDefault<T>()} {}
  T get() const {
    return value;
  }
  void set(T const& val) {
    changed = true;
    value = val;
  }
  void unset() {
    changed = false;
    value = getDefault<T>();
  }
  bool is_default() const {
    return !changed;
  }
 private:
  bool changed{false};
  T value;
};

template<typename T>
using OptionalParam = std::optional<T>;

template <typename Element>
inline void setNamedOptionHelper(Element& /*element*/) {}

template <typename Element, typename T, typename... Parameters>
void setNamedOptionHelper(Element& element, const T& v, Parameters... args) {
  element.set(v);
  setNamedOptionHelper(element, std::forward<Parameters>(args)...);
}

template <typename Element, typename T>
void setNamedOptionHelper(Element& element, const T& v) {
  element.set(v);
}

template<typename... Parameters>
class ElementImpl {
 private:
  std::tuple<Parameters...> params;

 public:
  using tuple_t = decltype(params);
  template<typename T>
  using has_optional_parameter = has_type<OptionalParam<T>, tuple_t>;
  template<typename T>
  using has_required_parameter = has_type<T, tuple_t>;
  template<typename T>
  using has_defaulted_parameter = has_type<DefaultedParam<T>, tuple_t>;

  template<typename T>
  using enable_if_has_optional = std::enable_if_t<has_optional_parameter<T>::value, bool>;
  template<typename T>
  using enable_if_has_required = std::enable_if_t<has_required_parameter<T>::value, bool>;
  template<typename T>
  using enable_if_has_defaulted = std::enable_if_t<has_defaulted_parameter<T>::value, bool>;

 public:
//  template <typename... Ts>
//  explicit ElementImpl(Ts...) {
//  }
  template<typename T, enable_if_has_required<T> = true>
  T get() const {
    return std::get<T>(params);
  }

  template<typename T, enable_if_has_optional<T> = true>
  T get() const {
    return *std::get<OptionalParam<T>>(params);
  }

  template<typename T, enable_if_has_defaulted<T> = true>
  T get() const {
    return std::get<DefaultedParam<T>>(params).get();
  }

  template<typename T, enable_if_has_required<T> = true>
  void set(T val) {
    std::get<T>(params) = val;
  }

  template<typename T, enable_if_has_optional<T> = true>
  void set(T val) {
    std::get<OptionalParam<T>>(params) = val;
  }

  template<typename T, enable_if_has_defaulted<T> = true>
  void set(T val) {
    std::get<DefaultedParam<T>>(params).set(val);
  }

  template<typename T, enable_if_has_required<T> = true>
  bool has() const {
    return true;
  }

  template<typename T, enable_if_has_optional<T> = true>
  bool has() const {
    return std::get<OptionalParam<T>>(params).has_value();
  }

  template<typename T, enable_if_has_defaulted<T> = true>
  bool has() const {
    return true;
  }

  template<typename T, enable_if_has_defaulted<T> = true>
  void unset() {
    std::get<DefaultedParam<T>>(params).unset();
  }

  template<typename T, enable_if_has_optional<T> = true>
  void unset() {
    std::get<OptionalParam<T>>(params) = OptionalParam<T>{};
  }

  template<typename T>
  bool isDefault() const {
    return std::get<DefaultedParam<T>>(params).is_default();
  }
};

template<typename Derived, typename DerivedImpl>
class ElementBase {
 private:
  ElementBase() = default;
  friend Derived;
 public:
  template <typename... Parameters>
  explicit ElementBase(Parameters... parameters) {
    auto& el = static_cast<Derived &>(*this);
    setNamedOptionHelper(el, std::forward<Parameters>(parameters)...);
  }
  template <typename T, std::enable_if_t<DerivedImpl::template has_optional_parameter<T>::value ||
                                         DerivedImpl::template has_defaulted_parameter<T>::value ||
                                         DerivedImpl::template has_required_parameter<T>::value, bool> = true>
//  template <typename T>
  T get() const {
    auto const& el = static_cast<Derived const&>(*this);
    return el.impl_.template get<T>();
  }

  template <typename T, std::enable_if_t<DerivedImpl::template has_optional_parameter<T>::value ||
                                         DerivedImpl::template has_defaulted_parameter<T>::value ||
                                         DerivedImpl::template has_required_parameter<T>::value, bool> = true>
  bool has() const {
    auto const& el = static_cast<Derived const&>(*this);
    return el.impl_.template has<T>();
  }

  template <typename T, std::enable_if_t<DerivedImpl::template has_defaulted_parameter<T>::value, bool> = true>
  bool isDefault() const {
    auto const& el = static_cast<Derived const&>(*this);
    return el.impl_.template isDefault<T>();
  }

  template <typename T, std::enable_if_t<DerivedImpl::template has_optional_parameter<T>::value ||
                                         DerivedImpl::template has_defaulted_parameter<T>::value, bool> = true>
  void unset() {
    auto& el = static_cast<Derived&>(*this);
    return el.impl_.template unset<T>();
  }

};
