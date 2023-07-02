#pragma once

#include <cstddef>
#include <iterator>

template <typename T>
class list {
  struct abstract_node {
    abstract_node() noexcept : prev(this), next(this) {}

    abstract_node* prev;
    abstract_node* next;
  };

  struct node : abstract_node {
    node(const T& value) : abstract_node(), value(value) {}

    T value;
  };

  template <class K>
  class list_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = K&;
    using pointer = K*;
    using iterator_category = std::bidirectional_iterator_tag;

  private:
    abstract_node* node_;

    list_iterator(abstract_node* node) noexcept : node_(node) {}

    friend list;

  public:
    list_iterator() = default;

    operator list_iterator<const K>() const noexcept {
      return list_iterator<const K>(node_);
    }

    reference operator*() const noexcept {
      return static_cast<node*>(node_)->value;
    }

    pointer operator->() const noexcept {
      return &(static_cast<node*>(node_)->value);
    }

    list_iterator& operator++() noexcept {
      node_ = node_->next;
      return *this;
    }

    list_iterator operator++(int) noexcept {
      list_iterator res = *this;
      ++(*this);
      return res;
    }

    list_iterator& operator--() noexcept {
      node_ = node_->prev;
      return *this;
    }

    list_iterator operator--(int) noexcept {
      list_iterator res = *this;
      --(*this);
      return res;
    }

    friend bool operator==(const list_iterator& left, const list_iterator& right) noexcept {
      return left.node_ == right.node_;
    }

    friend bool operator!=(const list_iterator& left, const list_iterator& right) noexcept {
      return !(left == right);
    }
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = list_iterator<T>;
  using const_iterator = list_iterator<const T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  size_t size_ = 0;
  abstract_node cycle_node_;

  void link(abstract_node* a, abstract_node* b) noexcept {
    a->next = b;
    b->prev = a;
  }

public:
  // O(1), nothrow
  list() noexcept = default;

  // O(n), strong
  list(const list& other) : list(other.begin(), other.end()) {}

  // O(n), strong
  template <std::input_iterator InputIt>
  list(InputIt first, InputIt last) : list() {
    for (std::input_iterator auto it = first; it != last; ++it) {
      push_back(*it);
    }
  }

  // O(n), strong
  list& operator=(const list& other) {
    if (this == &other) {
      return *this;
    }
    list copy(other);
    swap(*this, copy);
    return *this;
  }

  // O(n), nothrow
  ~list() noexcept {
    clear();
  }

  // O(1), nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1), nothrow
  size_t size() const noexcept {
    return size_;
  }

  // O(1), nothrow
  T& front() {
    return *(begin());
  }

  // O(1), nothrow
  const T& front() const {
    return *(begin());
  }

  // O(1), nothrow
  T& back() {
    return *(--end());
  }

  // O(1), nothrow
  const T& back() const {
    return *(--end());
  }

  // O(1), strong
  void push_front(const T& e) {
    insert(begin(), e);
  }

  // O(1), strongs
  void push_back(const T& e) {
    insert(end(), e);
  }

  // O(1), nothrow
  void pop_front() {
    erase(begin());
  }

  // O(1), nothrow
  void pop_back() {
    erase(std::prev(end()));
  }

  // O(1), nothrow
  iterator begin() noexcept {
    return iterator(cycle_node_.next);
  }

  // O(1), nothrow
  const_iterator begin() const noexcept {
    return const_iterator(cycle_node_.next);
  }

  // O(1), nothrow
  iterator end() noexcept {
    return iterator(&cycle_node_);
  }

  // O(1), nothrow
  const_iterator end() const noexcept {
    return const_iterator(const_cast<abstract_node*>(&cycle_node_));
  }

  // O(1), nothrow
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  // O(1), nothrow
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  // O(1), nothrow
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  // O(1), nothrow
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(n), nothrow
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), strong
  iterator insert(const_iterator pos, const T& e) {
    abstract_node* at_pos_prev = pos.node_->prev;
    abstract_node* new_node = new node(e);
    link(new_node, pos.node_);
    link(at_pos_prev, new_node);
    ++size_;
    return iterator(new_node);
  }

  // O(last - first), strong
  template <std::input_iterator InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    if (first == last) {
      return iterator(pos.node_);
    }
    list tmp(first, last);
    auto ret = tmp.begin();
    splice(pos, tmp, tmp.begin(), tmp.end());
    return ret;
  }

  // O(1), nothrow
  iterator erase(const_iterator pos) noexcept {
    abstract_node* bef_pos = pos.node_->prev;
    abstract_node* next_to_pos = pos.node_->next;
    link(bef_pos, next_to_pos);
    delete static_cast<node*>(pos.node_);
    --size_;
    return iterator(next_to_pos);
  }

  // O(last - first), nothrow
  iterator erase(const_iterator first, const_iterator last) noexcept {
    list tmp;
    tmp.splice(tmp.end(), *this, first, last);
    return iterator(last.node_);
  }

  // O(last - first) in general but O(1) when possible, nothrow
  void splice(const_iterator pos, list& other, const_iterator first, const_iterator last) noexcept {
    if (last == first) {
      return;
    }
    if (this != &other) {
      std::ptrdiff_t dist;
      if (first == other.begin() && last == other.end()) {
        dist = other.size();
      } else {
        dist = std::distance(first, last);
      }
      size_ += dist;
      other.size_ -= dist;
    }
    abstract_node* current = pos.node_;
    abstract_node* before_cur = pos.node_->prev;
    abstract_node* last_elem = last.node_;
    abstract_node* new_begin = first.node_->prev;
    link(last_elem->prev, current);
    link(new_begin, last_elem);
    link(before_cur, first.node_);
  }

  // O(1), nothrow
  friend void swap(list& left, list& right) noexcept {
    abstract_node* lf_prev = left.cycle_node_.prev;
    abstract_node* lf_next = left.cycle_node_.next;
    abstract_node* rh_prev = right.cycle_node_.prev;
    abstract_node* rh_next = right.cycle_node_.next;

    lf_prev->next = lf_next->prev = &right.cycle_node_;
    rh_prev->next = rh_next->prev = &left.cycle_node_;

    std::swap(left.cycle_node_, right.cycle_node_);
    std::swap(left.size_, right.size_);
  }
};
