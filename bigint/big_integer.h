#pragma once

#include <algorithm>
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>

struct big_integer {
public:
  using data_type = unsigned int;

private:
  static const uint64_t LIMIT = static_cast<uint64_t>(std::numeric_limits<data_type>::max()) + 1;
  static const uint8_t BITS_COUNT = 32;
  static const uint32_t STR_NUMS = 1'000'000'000;
  static const uint32_t STR_NUMS_COUNT = 9;

  std::vector<data_type> data_;
  int8_t sign;
  big_integer divide(const big_integer& b);
  uint64_t divModByConst(uint64_t rhs);
  void swap(big_integer& other) noexcept;
  void deleteLeadingZeroes();
  int normalize();
  void make(uint64_t value);
  void invert();
  void back_invert();
  void divByConst(uint64_t rhs);
  void mulByConst(uint64_t rhs);
  void subtract(const big_integer& rhs);
  bool isZero() const noexcept;
  template <typename Operation>
  void bitwiseOp(const big_integer& other, Operation op);

public:
  big_integer() noexcept;
  big_integer(const big_integer& other);
  big_integer(int a);
  big_integer(unsigned int a);
  big_integer(long a);
  big_integer(unsigned long a);
  big_integer(long long a);
  big_integer(unsigned long long a);
  explicit big_integer(const std::string& str);
  ~big_integer();

  big_integer& operator=(const big_integer& other) noexcept;
  big_integer& operator+=(const big_integer& rhs);
  big_integer& operator+=(uint64_t rhs);
  big_integer& operator-=(const big_integer& rhs);
  big_integer& operator*=(const big_integer& rhs);
  big_integer& operator/=(const big_integer& rhs);
  big_integer& operator%=(const big_integer& rhs);

  big_integer& operator&=(const big_integer& rhs);
  big_integer& operator|=(const big_integer& rhs);
  big_integer& operator^=(const big_integer& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const noexcept;
  big_integer operator-() const noexcept;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(const big_integer& a, const big_integer& b) noexcept;
  friend bool operator!=(const big_integer& a, const big_integer& b) noexcept;
  friend bool operator<(const big_integer& a, const big_integer& b) noexcept;
  friend bool operator>(const big_integer& a, const big_integer& b) noexcept;
  friend bool operator<=(const big_integer& a, const big_integer& b) noexcept;
  friend bool operator>=(const big_integer& a, const big_integer& b) noexcept;

  friend std::string to_string(const big_integer& a);
};

big_integer operator+(const big_integer& a, const big_integer& b);
big_integer operator-(const big_integer& a, const big_integer& b);
big_integer operator*(const big_integer& a, const big_integer& b);
big_integer operator/(const big_integer& a, const big_integer& b);
big_integer operator%(const big_integer& a, const big_integer& b);

big_integer operator&(const big_integer& a, const big_integer& b);
big_integer operator|(const big_integer& a, const big_integer& b);
big_integer operator^(const big_integer& a, const big_integer& b);

big_integer operator<<(const big_integer& a, int b);
big_integer operator>>(const big_integer& a, int b);

bool operator==(const big_integer& a, const big_integer& b) noexcept;
bool operator!=(const big_integer& a, const big_integer& b) noexcept;
bool operator<(const big_integer& a, const big_integer& b) noexcept;
bool operator>(const big_integer& a, const big_integer& b) noexcept;
bool operator<=(const big_integer& a, const big_integer& b) noexcept;
bool operator>=(const big_integer& a, const big_integer& b) noexcept;

std::string to_string(const big_integer& a);
std::ostream& operator<<(std::ostream& out, const big_integer& a);
