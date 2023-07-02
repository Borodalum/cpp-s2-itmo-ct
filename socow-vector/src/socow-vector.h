#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

template <typename T, size_t SMALL_SIZE>
class socow_vector {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

private:
  struct buffer {
    size_t ref_count_;
    size_t capacity_;
    value_type data_[0];

    bool unique() {
      return ref_count_ == 1;
    }
  };

  bool isSmall = true;
  size_t size_ = 0;

  union {
    value_type small_data_[SMALL_SIZE];
    buffer* big_data_;
  };

  buffer* create_buf(size_t capacity) {
    auto new_data = static_cast<buffer*>(operator new(sizeof(buffer) + capacity * sizeof(T)));
    new (new_data) buffer{1, capacity, {}};
    return new_data;
  }

  void delete_buf(buffer* buf) {
    buf->~buffer();
    operator delete(buf);
  }

  void create_and_copy_new(size_t size, size_t capacity, const_pointer source) {
    socow_vector tmp(capacity);
    for (size_t i = 0; i < size; ++i) {
      tmp.push_back(source[i]);
    }
    *this = tmp;
  }

  socow_vector(size_t initial_capacity) {
    if (initial_capacity == 0 || initial_capacity <= SMALL_SIZE) {
      return;
    }
    buffer* new_data = create_buf(initial_capacity);
    big_data_ = new_data;
    isSmall = false;
  }

  socow_vector(const socow_vector& other, size_t capacity) {
    if (capacity == 0) {
      return;
    }
    if (capacity <= SMALL_SIZE) {
      std::uninitialized_copy(other.data(), other.end(), data());
      size_ = other.size();
      return;
    }
    socow_vector tmp(capacity);
    for (size_t i = 0; i < other.size(); ++i) {
      tmp.push_back(other[i]);
    }
    *this = tmp;
  }

  void shrink_to_small() {
    buffer* tmp = big_data_;
    try {
      std::uninitialized_copy_n(tmp->data_, size(), small_data_);
      release_ref(tmp);
    } catch (...) {
      big_data_ = tmp;
      throw;
    }
    isSmall = true;
  }

  void release_ref(buffer* buf) {
    --buf->ref_count_;
    if (buf->ref_count_ == 0) {
      std::destroy_n(buf->data_, size());
      delete_buf(buf);
    }
  }

  void unshare() {
    if (big_data_->unique()) {
      return;
    }
    create_and_copy_new(size(), capacity(), big_data_->data_);
  }

  bool is_shared() {
    return (!isSmall && big_data_->ref_count_ > 1);
  }

  bool big_unique() {
    return (!isSmall && big_data_->ref_count_ == 1);
  }

  void swap_two_small(socow_vector& a, socow_vector& b) {
    if (a.size() > b.size()) {
      swap_two_small(b, a);
      return;
    }
    size_t common_len = a.size();
    size_t diff_len = b.size() - a.size();
    std::uninitialized_copy_n(b.data() + common_len, diff_len, a.data() + common_len);
    try {
      std::swap_ranges(a.begin(), a.begin() + common_len, b.begin());
    } catch (...) {
      std::destroy_n(a.data() + common_len, diff_len);
      throw;
    }
    std::destroy_n(b.data() + common_len, diff_len);
    std::swap(a.size_, b.size_);
  }

  void swap_small_big(socow_vector& small, socow_vector& big) {
    buffer* tmp = big.big_data_;
    try {
      std::uninitialized_copy_n(small.small_data_, small.size(), big.small_data_);
    } catch (...) {
      big.big_data_ = tmp;
      throw;
    }
    std::destroy_n(small.small_data_, small.size());
    std::swap(small.isSmall, big.isSmall);
    std::swap(small.size_, big.size_);
    small.big_data_ = tmp;
  }

public:
  socow_vector() {}

  socow_vector(const socow_vector& other) : isSmall(other.isSmall), size_(other.size()) {
    if (other.isSmall) {
      std::uninitialized_copy_n(other.data(), other.size(), small_data_);
    } else {
      big_data_ = other.big_data_;
      ++big_data_->ref_count_;
    }
  }

  socow_vector& operator=(const socow_vector& other) {
    if (this == &other) {
      return *this;
    }
    if (isSmall && other.isSmall) {
      size_t common_len = std::min(size(), other.size());
      socow_vector tmp;
      for (size_t i = 0; i < common_len; ++i) {
        tmp.push_back(other[i]);
      }
      if (size() < other.size()) {
        std::uninitialized_copy_n(other.data() + size(), other.size() - size(), data() + size());
      } else if (other.size() < size()) {
        std::destroy_n(data() + other.size(), size() - other.size());
      }
      size_ = other.size();
      std::swap_ranges(tmp.begin(), tmp.end(), data());
    } else if (isSmall) {
      std::destroy_n(small_data_, size());
      big_data_ = other.big_data_;
      ++big_data_->ref_count_;
    } else if (other.isSmall) {
      buffer* tmp = big_data_;
      try {
        std::uninitialized_copy_n(other.small_data_, other.size(), small_data_);
      } catch (...) {
        big_data_ = tmp;
        throw;
      }
      release_ref(tmp);
    } else {
      release_ref(big_data_);
      big_data_ = other.big_data_;
      ++big_data_->ref_count_;
    }
    size_ = other.size();
    isSmall = other.isSmall;
    return *this;
  }

  ~socow_vector() {
    if (isSmall) {
      std::destroy_n(small_data_, size());
    } else {
      release_ref(big_data_);
    }
  }

  void swap(socow_vector& other) {
    if (&other == this) {
      return;
    }
    if (isSmall && other.isSmall) {
      swap_two_small(*this, other);
    } else if (isSmall) {
      swap_small_big(*this, other);
    } else if (other.isSmall) {
      swap_small_big(other, *this);
    } else {
      std::swap(size_, other.size_);
      std::swap(big_data_, other.big_data_);
    }
  }

  void push_back(const T& e) {
    size_t new_capacity = (capacity() == 0) ? 1 : capacity() * 2;
    if (size() < capacity()) {
      if (isSmall || big_unique()) {
        new (data() + size()) T(e);
        ++size_;
        return;
      }
      new_capacity = capacity();
    }
    socow_vector tmp(*this, new_capacity);
    tmp.push_back(e);
    *this = tmp;
  }

  void pop_back() {
    if (isSmall || big_unique()) {
      back().~T();
      --size_;
    } else {
      create_and_copy_new(size() - 1, capacity(), big_data_->data_);
    }
  }

  iterator insert(const_iterator pos, const T& value) {
    size_t rangeFromBegin = pos - cbegin();
    if (size() == capacity() || is_shared()) {
      socow_vector temp(capacity() * 2);
      size_t i = 0;
      for (; i < rangeFromBegin; ++i) {
        temp.push_back(cbegin()[i]);
      }
      temp.push_back(value);
      for (; i < size(); ++i) {
        temp.push_back(cbegin()[i]);
      }
      *this = temp;
    } else {
      push_back(value);
      for (iterator it = end() - 1; it != begin() + rangeFromBegin; --it) {
        std::iter_swap(it, it - 1);
      }
    }
    return begin() + rangeFromBegin;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    size_t range = last - first;
    size_t rangeFromBegin = first - cbegin();
    if (first == last) {
      return begin() + rangeFromBegin;
    }
    if (is_shared()) {
      socow_vector temp(capacity() * 2);
      size_t i = 0;
      for (; i < rangeFromBegin; ++i) {
        temp.push_back(big_data_->data_[i]);
      }
      i += range;
      for (; i < size(); ++i) {
        temp.push_back(big_data_->data_[i]);
      }
      *this = temp;
    } else {
      std::swap_ranges(begin() + rangeFromBegin, end() - range, begin() + rangeFromBegin + range);
      std::destroy_n(cend() - range, range);
      size_ -= range;
    }
    return begin() + rangeFromBegin;
  }

  reference operator[](size_t index) {
    return data()[index];
  }

  const_reference operator[](size_t index) const noexcept {
    return data()[index];
  }

  reference front() {
    return data()[0];
  }

  const_reference front() const noexcept {
    return data()[0];
  }

  reference back() {
    return data()[size() - 1];
  }

  const_reference back() const noexcept {
    return data()[size() - 1];
  }

  size_t size() const noexcept {
    return size_;
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  size_t capacity() const noexcept {
    if (isSmall) {
      return SMALL_SIZE;
    } else {
      return big_data_->capacity_;
    }
  }

  pointer data() {
    if (isSmall) {
      return small_data_;
    } else {
      unshare();
      return big_data_->data_;
    }
  }

  const_pointer data() const noexcept {
    if (isSmall) {
      return small_data_;
    } else {
      return big_data_->data_;
    }
  }

  void reserve(size_t new_capacity) {
    if ((new_capacity <= capacity() && (isSmall || big_data_->unique())) || new_capacity < size()) {
      return;
    }
    if (new_capacity <= SMALL_SIZE) {
      shrink_to_small();
      return;
    }
    *this = socow_vector(*this, new_capacity);
  }

  void shrink_to_fit() {
    if (size() == capacity() || isSmall) {
      return;
    }
    if (size() <= SMALL_SIZE) {
      shrink_to_small();
      return;
    }
    *this = socow_vector(*this, size());
  }

  void clear() {
    if (is_shared()) {
      socow_vector tmp(capacity());
      *this = tmp;
    } else {
      std::destroy_n(data(), size());
    }
    size_ = 0;
  }

  iterator begin() {
    return data();
  }

  iterator end() {
    return data() + size();
  }

  const_iterator cbegin() const {
    return std::as_const(*this).begin();
  }

  const_iterator cend() const {
    return std::as_const(*this).end();
  }

  const_iterator begin() const noexcept {
    return data();
  }

  const_iterator end() const noexcept {
    return data() + size();
  }
};
