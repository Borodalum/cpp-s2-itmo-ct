#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>

template <class T>
class matrix {
  template <typename K>
  class col_base_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = K&;
    using pointer = K*;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer element_;
    size_t col_count_;
    size_t offset_;

    explicit col_base_iterator(pointer elem, size_t col_count, size_t offset)
        : element_(elem),
          col_count_(col_count),
          offset_(offset) {}

    friend matrix;

  public:
    col_base_iterator() = default;

    operator col_base_iterator<const K>() const {
      return col_base_iterator<const K>(element_, col_count_, offset_);
    }

    reference operator*() const {
      return element_[offset_];
    }

    pointer operator->() const {
      return element_ + offset_;
    }

    reference operator[](const difference_type ind) const {
      difference_type index = ind * col_count_;
      return element_[static_cast<difference_type>(offset_) + index];
    }

    col_base_iterator& operator++() {
      element_ += col_count_;
      return *this;
    }

    col_base_iterator operator++(int) {
      col_base_iterator res = *this;
      ++(*this);
      return res;
    }

    col_base_iterator& operator--() {
      element_ -= col_count_;
      return *this;
    }

    col_base_iterator operator--(int) {
      col_base_iterator res = *this;
      --(*this);
      return res;
    }

    col_base_iterator& operator+=(const difference_type factor) {
      element_ += factor * static_cast<difference_type>(col_count_);
      return *this;
    }

    friend col_base_iterator operator+(const col_base_iterator& left, const difference_type right) {
      col_base_iterator out(left.element_, left.col_count_, left.offset_);
      out += right;
      return out;
    }

    friend col_base_iterator operator+(const difference_type left, const col_base_iterator& right) {
      return right + left;
    }

    col_base_iterator& operator-=(const difference_type factor) {
      element_ -= factor * static_cast<difference_type>(col_count_);
      return *this;
    }

    friend col_base_iterator operator-(const col_base_iterator& left, const difference_type right) {
      col_base_iterator out(left.element_, left.col_count_, left.offset_);
      out -= right;
      return out;
    }

    friend difference_type operator-(const col_base_iterator& left, const col_base_iterator& right) {
      return (left.element_ - right.element_) / static_cast<difference_type>(left.col_count_);
    }

    friend bool operator==(const col_base_iterator& left, const col_base_iterator& right) {
      return left.element_ == right.element_ && left.offset_ == right.offset_;
    }

    friend bool operator!=(const col_base_iterator& left, const col_base_iterator& right) {
      return !(left == right);
    }

    friend bool operator>(const col_base_iterator& left, const col_base_iterator& right) {
      return left.element_ > right.element_ && left.offset_ == right.offset_;
    }

    friend bool operator>=(const col_base_iterator& left, const col_base_iterator& right) {
      return left.element_ >= right.element_ && left.offset_ == right.offset_;
    }

    friend bool operator<(const col_base_iterator& left, const col_base_iterator& right) {
      return left.element_ < right.element_ && left.offset_ == right.offset_;
    }

    friend bool operator<=(const col_base_iterator& left, const col_base_iterator& right) {
      return left.element_ <= right.element_ && left.offset_ == right.offset_;
    }
  };

private:
  T* data_;
  size_t rows_;
  size_t cols_;

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = T*;
  using const_iterator = const T*;

  using row_iterator = T*;
  using const_row_iterator = const T*;

  using col_iterator = col_base_iterator<T>;
  using const_col_iterator = col_base_iterator<const T>;

public:
  matrix() : data_(nullptr), rows_(0), cols_(0) {}

  matrix(size_t rows, size_t cols)
      : data_(rows * cols == 0 ? nullptr : new T[rows * cols]()),
        rows_(rows * cols == 0 ? 0 : rows),
        cols_(rows * cols == 0 ? 0 : cols) {}

  template <size_t Rows, size_t Cols>
  matrix(const T (&init)[Rows][Cols]) : data_(new T[Rows * Cols]),
                                        rows_(Rows),
                                        cols_(Cols) {
    iterator it = begin();
    for (size_t i = 0; i < rows(); i++) {
      it = std::copy_n(init[i], cols_, it);
    }
  }

  matrix(const matrix& other)
      : data_(other.size() == 0 ? nullptr : new T[other.size()]),
        rows_(other.size() == 0 ? 0 : other.rows()),
        cols_(other.size() == 0 ? 0 : other.cols()) {
    std::copy(other.begin(), other.end(), begin());
  }

  matrix& operator=(const matrix& other) {
    if (this == &other) {
      return *this;
    }
    matrix(other).swap(*this);
    return *this;
  }

  void swap(matrix& other) {
    std::swap(data_, other.data_);
    std::swap(rows_, other.rows_);
    std::swap(cols_, other.cols_);
  }

  ~matrix() {
    delete[] data_;
  }

  // Iterators

  iterator begin() {
    return data();
  }

  const_iterator begin() const {
    return data();
  }

  iterator end() {
    return begin() + size();
  }

  const_iterator end() const {
    return begin() + size();
  }

  row_iterator row_begin(size_t ind) {
    return data() + ind * cols();
  }

  const_row_iterator row_begin(size_t ind) const {
    return data() + ind * cols();
  }

  row_iterator row_end(size_t ind) {
    return row_begin(ind) + cols();
  }

  const_row_iterator row_end(size_t ind) const {
    return row_begin(ind) + cols();
  }

  col_iterator col_begin(size_t ind) {
    return col_iterator(data(), cols(), ind);
  }

  const_col_iterator col_begin(size_t ind) const {
    return const_col_iterator(data(), cols(), ind);
  }

  col_iterator col_end(size_t ind) {
    return col_begin(ind) + rows();
  }

  const_col_iterator col_end(size_t ind) const {
    return col_begin(ind) + rows();
  }

  // Size

  size_t rows() const {
    return rows_;
  }

  size_t cols() const {
    return cols_;
  }

  size_t size() const {
    return rows_ * cols_;
  }

  bool empty() const {
    return size() == 0;
  }

  // Elements access

  reference operator()(size_t row, size_t col) {
    return data_[row * cols() + col];
  }

  const_reference operator()(size_t row, size_t col) const {
    return data_[row * cols() + col];
  }

  pointer data() {
    return data_;
  }

  const_pointer data() const {
    return data_;
  }

  // Comparison

  friend bool operator==(const matrix& left, const matrix& right) {
    if (left.cols() != right.cols() || left.rows() != right.rows()) {
      return false;
    }
    for (size_t i = 0; i < left.rows(); ++i) {
      for (size_t j = 0; j < left.cols(); ++j) {
        if (left(i, j) != right(i, j)) {
          return false;
        }
      }
    }
    return true;
  }

  friend bool operator!=(const matrix& left, const matrix& right) {
    return !(left == right);
  }

  // Arithmetic operations

  matrix& operator+=(const matrix& other) {
    std::transform(begin(), end(), other.begin(), begin(), std::plus<>{});
    return *this;
  }

  matrix& operator-=(const matrix& other) {
    std::transform(begin(), end(), other.begin(), begin(), std::minus<>{});
    return *this;
  }

  matrix& operator*=(const matrix& other) {
    matrix<value_type> out(rows(), other.cols());
    for (size_t i = 0; i < rows(); ++i) {
      for (size_t j = 0; j < other.cols(); ++j) {
        out(i, j) = 0;
        for (size_t k = 0; k < cols(); ++k) {
          out(i, j) += (*this)(i, k) * other(k, j);
        }
      }
    }
    *this = out;
    return *this;
  }

  matrix& operator*=(const_reference factor) {
    std::transform(begin(), end(), begin(), [&factor](T a) { return a * factor; });
    return *this;
  }

  friend matrix operator+(const matrix& left, const matrix& right) {
    matrix out = left;
    out += right;
    return out;
  }

  friend matrix operator-(const matrix& left, const matrix& right) {
    matrix out = left;
    out -= right;
    return out;
  }

  friend matrix operator*(const matrix& left, const matrix& right) {
    matrix out = left;
    out *= right;
    return out;
  }

  friend matrix operator*(const matrix& left, const_reference right) {
    matrix out = left;
    out *= right;
    return out;
  }

  friend matrix operator*(const_reference left, const matrix& right) {
    return right * left;
  }
};
