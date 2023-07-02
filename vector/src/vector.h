#pragma once

#include <cstddef>

template <typename T>
class vector {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

private:
  pointer data_;
  size_t size_;
  size_t capacity_;

  vector(const vector& other, size_t new_capacity) : data_{nullptr}, size_{other.size()}, capacity_{new_capacity} {
    assert(other.size() <= new_capacity);
    if (new_capacity == 0) {
      return;
    }
    data_ = static_cast<T*>(operator new(sizeof(T) * new_capacity));
    size_t i = 0;
    try {
      for (; i < other.size(); ++i) {
        new (data_ + i) T(other[i]);
      }
    } catch (...) {
      for (size_t j = i; j > 0; --j) {
        (data_ + j - 1)->~T();
      }
      operator delete(data_);
      throw;
    }
    capacity_ = new_capacity;
    size_ = other.size();
  }

public:
  vector() noexcept : data_{nullptr}, size_{0}, capacity_{0} {}

  vector(const vector& other) : vector(other, other.size()) {}

  void swap(vector& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  vector& operator=(const vector& other) {
    if (&other != this) {
      vector(other).swap(*this);
    }
    return *this;
  }

  ~vector() noexcept {
    clear();
    operator delete(data_);
  }

  reference operator[](size_t index) {
    return data_[index];
  }

  const_reference operator[](size_t index) const {
    return data_[index];
  }

  pointer data() noexcept {
    return data_;
  }

  const_pointer data() const noexcept {
    return data_;
  }

  size_t size() const noexcept {
    return size_;
  }

  reference front() {
    return data_[0];
  }

  const_reference front() const {
    return data_[0];
  }

  reference back() {
    assert(size() > 0);
    return data_[size() - 1];
  }

  const_reference back() const {
    assert(size() > 0);
    return data_[size() - 1];
  }

  void push_back(const T& e) {
    if (size() + 1 <= capacity()) {
      new (data_ + size()) T(e);
      ++size_;
      return;
    }
    size_t new_capacity = (capacity() == 0) ? 1 : capacity() * 2;
    vector tmp(*this, new_capacity);
    tmp.push_back(e);
    swap(tmp);
  }

  void pop_back() {
    back().~T();
    --size_;
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  size_t capacity() const noexcept {
    return capacity_;
  }

  void reserve(size_t new_capacity) {
    if (new_capacity > capacity()) {
      vector(*this, new_capacity).swap(*this);
    }
  }

  void shrink_to_fit() {
    if (size() != capacity()) {
      vector(*this, size()).swap(*this);
    }
  }

  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  iterator begin() noexcept {
    return data();
  }

  iterator end() noexcept {
    return data() + size();
  }

  const_iterator begin() const noexcept {
    return data();
  }

  const_iterator end() const noexcept {
    return data() + size();
  }

  iterator insert(const_iterator pos, const T& value) {
    size_t rangeFromBegin = pos - begin();
    push_back(value);
    for (iterator it = end() - 1; it != begin() + rangeFromBegin; --it) {
      std::iter_swap(it, it - 1);
    }
    return begin() + rangeFromBegin;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    size_t range = last - first;
    size_t rangeFromBegin = first - begin();
    for (iterator it = begin() + rangeFromBegin; it < end() - range; ++it) {
      std::iter_swap(it, it + range);
    }
    for (size_t i = 0; i < range; ++i) {
      pop_back();
    }
    return begin() + rangeFromBegin;
  }
};
