#include "big_integer.h"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <vector>

bool big_integer::isZero() const noexcept {
  return data_.empty() || (data_.size() == 1 && data_.back() == 0);
}

big_integer::big_integer() noexcept : sign(1) {}

big_integer::big_integer(const big_integer& other) = default;

void big_integer::make(uint64_t value) {
  if (value == 0) {
    return;
  }
  while (value > 0) {
    data_.push_back(value);
    value >>= BITS_COUNT;
  }
}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned int a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long long a) : sign(a < 0 ? -1 : 1) {
  uint64_t value = 0;
  if (a < 0) {
    if (a == std::numeric_limits<long long>::min()) {
      value = std::numeric_limits<long long>::max();
      value += 1;
    } else {
      value = -a;
    }
  } else {
    value = a;
  }
  make(value);
}

big_integer::big_integer(unsigned long long a) : sign(1) {
  make(a);
}

big_integer::big_integer(const std::string& str) : big_integer() {
  if (str.empty() || (str.length() == 1 && str[0] == '-') || !std::all_of(str.begin() + 1, str.end(), ::isdigit) ||
      (str[0] != '-' && !::isdigit(str[0]))) {
    throw std::invalid_argument("biginteger can't contains non-integer values");
  }
  size_t i = 0;
  if (str[i] == '-') {
    sign = -1;
    i++;
  }
  for (; i < str.length(); i += STR_NUMS_COUNT) {
    uint32_t numbOfChars = std::min(str.length() - i, static_cast<unsigned long>(STR_NUMS_COUNT));
    uint32_t num = std::stoi(str.substr(i, numbOfChars));
    mulByConst(numbOfChars == STR_NUMS_COUNT ? STR_NUMS : std::pow(10, numbOfChars));
    *this += num;
  }
  deleteLeadingZeroes();
}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(const big_integer& other) noexcept {
  if (other == *this) {
    return *this;
  }
  big_integer(other).swap(*this);
  return *this;
}

void big_integer::swap(big_integer& other) noexcept {
  std::swap(other.sign, sign);
  std::swap(other.data_, data_);
}

void big_integer::deleteLeadingZeroes() {
  while (data_.size() > 1 && data_.back() == 0) {
    data_.pop_back();
  }
}

int big_integer::normalize() {
  int k = 0;
  data_type back = data_.back();
  while (back < LIMIT / 2) {
    back <<= 1;
    ++k;
  }
  *this <<= k;
  return k;
}

big_integer& big_integer::operator+=(uint64_t rhs) {
  data_.resize(data_.size() + 1);
  uint64_t first_val = rhs + data_[0];
  data_[0] = first_val;
  data_type carry = (first_val >> BITS_COUNT);
  for (size_t i = 1; i < data_.size(); ++i) {
    uint64_t val = static_cast<uint64_t>(data_[i]) + carry;
    data_[i] = val;
    carry = (val >> BITS_COUNT);
  }
  deleteLeadingZeroes();
  return *this;
}

void big_integer::subtract(const big_integer& rhs) {
  bool greater = false;
  int8_t tmp_sign = sign;
  sign = rhs.sign;
  if ((rhs.sign == 1 && *this < rhs) || (rhs.sign == -1 && *this > rhs)) {
    greater = true;
    tmp_sign = rhs.sign;
  }
  size_t res_size = std::max(data_.size(), rhs.data_.size()) + 1;
  data_.resize(res_size);
  uint64_t carry = 0;
  for (size_t i = 0; i < res_size; ++i) {
    uint64_t first = (i < data_.size() ? data_[i] : 0);
    uint64_t second = (i < rhs.data_.size() ? rhs.data_[i] : 0);
    if (greater) {
      first = (i < rhs.data_.size() ? rhs.data_[i] : 0);
      second = (i < data_.size() ? data_[i] : 0);
    }
    uint64_t res = LIMIT + first;
    res -= carry + second;
    data_[i] = res;
    carry = (res >> BITS_COUNT) ^ 1;
  }
  sign = tmp_sign;
  deleteLeadingZeroes();
}

big_integer& big_integer::operator+=(const big_integer& rhs) {
  if (sign == rhs.sign) {
    size_t res_size = std::max(data_.size(), rhs.data_.size()) + 1;
    data_.resize(res_size);
    size_t carry = 0;
    for (size_t i = 0; i < res_size; ++i) {
      size_t first = i < data_.size() ? data_[i] : 0;
      size_t second = i < rhs.data_.size() ? rhs.data_[i] : 0;
      size_t res = first + second + carry;
      data_[i] = res;
      carry = (res >> BITS_COUNT);
    }
    deleteLeadingZeroes();
    return *this;
  }
  subtract(rhs);
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& rhs) {
  int8_t tmp_sign = 0;
  if (sign == rhs.sign) {
    if (*this > rhs) {
      tmp_sign = 1;
    } else {
      tmp_sign = -1;
    }
    subtract(rhs);
  } else {
    if (sign < rhs.sign) {
      tmp_sign = -1;
    } else {
      tmp_sign = 1;
    }
    sign = rhs.sign;
    *this += rhs;
  }
  sign = tmp_sign;
  return *this;
}

void big_integer::mulByConst(const uint64_t rhs) {
  uint64_t carry = 0;
  data_.resize(data_.size() + 1);
  for (data_type& i : data_) {
    uint64_t val = i * rhs + carry;
    i = val;
    carry = (val >> BITS_COUNT);
  }
  deleteLeadingZeroes();
}
big_integer& big_integer::operator*=(const big_integer& rhs) {
  data_.insert(data_.begin(), rhs.data_.size(), 0);
  for (size_t i = 0; i < data_.size() - rhs.data_.size(); ++i) {
    uint64_t carry = 0;
    uint64_t multipliyer = data_[rhs.data_.size() + i];
    data_[rhs.data_.size() + i] = 0;
    for (size_t j = 0; j < rhs.data_.size(); ++j) {
      uint64_t value = multipliyer * rhs.data_[j];
      value += data_[i + j] + carry;
      data_[i + j] = value;
      carry = value >> BITS_COUNT;
    }
    data_[rhs.data_.size() + i] = carry;
  }
  deleteLeadingZeroes();
  sign *= rhs.sign;
  return *this;
}

big_integer big_integer::divide(const big_integer& b) {
  if (isZero()) {
    return {};
  }
  if (b.isZero()) {
    throw std::runtime_error("dividing by zero");
  }
  big_integer copy_a = *this;
  big_integer copy_b = b;
  big_integer normalized_b = b;
  big_integer quotient;
  copy_a.sign = copy_b.sign = normalized_b.sign = 1;
  size_t shift = normalized_b.normalize();
  copy_a <<= shift;
  if (copy_a.data_.size() < normalized_b.data_.size()) {
    return {};
  }
  size_t k = copy_a.data_.size() - normalized_b.data_.size();
  quotient.data_.resize(k + 1);
  copy_b = normalized_b;
  copy_b.data_.insert(copy_b.data_.begin(), k, 0);
  if (copy_a >= copy_b) {
    quotient.data_[k] = 1;
    copy_a -= copy_b;
  } else {
    quotient.data_[k] = 0;
  }
  for (size_t j = k; j > 0; --j) {
    uint64_t q_tmp = 0;
    if (copy_a.data_.size() > 1) {
      q_tmp = (copy_a.data_.back() * LIMIT + copy_a.data_[copy_a.data_.size() - 2]) / normalized_b.data_.back();
    }
    data_type q_j = std::min(q_tmp, LIMIT - 1);
    copy_b = normalized_b;
    copy_b.data_.insert(copy_b.data_.begin(), j - 1, 0);
    big_integer b_mqj = copy_b;
    b_mqj.mulByConst(q_j);
    copy_a -= b_mqj;
    while (copy_a < 0) {
      --q_j;
      copy_a += copy_b;
    }
    quotient.data_[j - 1] = q_j;
  }
  if (!copy_a.isZero()) {
    copy_a >>= shift;
  }
  copy_a.sign = sign;
  quotient.sign = sign * b.sign;
  copy_a.deleteLeadingZeroes();
  quotient.deleteLeadingZeroes();
  *this = copy_a;
  return quotient;
}

void big_integer::divByConst(const uint64_t rhs) {
  std::uint64_t carry = 0;
  for (size_t i = data_.size(); i > 0; --i) {
    std::uint64_t curr = data_[i - 1] + carry * LIMIT;
    data_[i - 1] = (curr / rhs);
    carry = (curr % rhs);
  }
  deleteLeadingZeroes();
}

big_integer& big_integer::operator/=(const big_integer& rhs) {
  *this = big_integer::divide(rhs);
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& rhs) {
  big_integer::divide(rhs);
  return *this;
}

void big_integer::invert() {
  if (sign > 0) {
    return;
  }
  for (data_type& i : data_) {
    i = ~i;
  }
  *this += 1;
}

void big_integer::back_invert() {
  sign = 1;
  *this -= 1;
  for (data_type& i : data_) {
    i = ~i;
  }
  sign = -1;
  deleteLeadingZeroes();
}

template <typename Operation>
void big_integer::bitwiseOp(const big_integer& other, Operation op) {
  invert();
  size_t beforeSize = data_.size();
  data_.resize(std::max(data_.size(), other.data_.size()));
  uint64_t carry = 1;
  for (size_t i = 0; i < data_.size(); ++i) {
    data_type first = i < beforeSize ? data_[i] : (sign < 0) ? (LIMIT - 1) : 0;
    data_type second = i < other.data_.size() ? other.data_[i] : (sign < 0) ? (LIMIT - 1) : 0;
    if (i < other.data_.size() && other.sign < 0) {
      second = ~other.data_[i];
      uint64_t with_carry = carry + second;
      second = with_carry;
      carry = (with_carry >> BITS_COUNT);
    }
    data_[i] = op(first, second);
  }
  deleteLeadingZeroes();
}

big_integer& big_integer::operator&=(const big_integer& rhs) {
  bitwiseOp(rhs, std::bit_and<>());
  if (sign < 0 && rhs.sign < 0) {
    back_invert();
  } else if (sign < 0 && rhs.sign > 0) {
    sign = 1;
  }
  return *this;
}

big_integer& big_integer::operator|=(const big_integer& rhs) {
  bitwiseOp(rhs, std::bit_or<>());
  if (sign < 0 || rhs.sign < 0) {
    back_invert();
  }
  return *this;
}

big_integer& big_integer::operator^=(const big_integer& rhs) {
  bitwiseOp(rhs, std::bit_xor<>());
  if ((sign < 0 && rhs.sign > 0) || (sign > 0 && rhs.sign < 0)) {
    back_invert();
  } else if (sign == rhs.sign) {
    sign = 1;
  }
  return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
  uint32_t shiftAbs = rhs / BITS_COUNT;
  data_.insert(data_.begin(), shiftAbs, 0);
  rhs %= BITS_COUNT;
  uint32_t value = 1 << rhs;
  mulByConst(value);
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  uint32_t shiftAbs = std::min(1UL * rhs / BITS_COUNT, data_.size());
  data_.erase(data_.begin(), data_.begin() + shiftAbs);
  if (data_.empty()) {
    *this = big_integer();
    return *this;
  }
  rhs %= BITS_COUNT;
  uint32_t value = (1 << rhs);
  divByConst(value);
  if (sign == -1) {
    *this += 1;
  }
  return *this;
}

big_integer big_integer::operator+() const noexcept {
  return *this;
}

big_integer big_integer::operator-() const noexcept {
  big_integer tmp = *this;
  tmp.sign = -sign;
  return tmp;
}

big_integer big_integer::operator~() const {
  big_integer copy = *this;
  copy.sign = -sign;
  copy += 1;
  return copy;
}

big_integer& big_integer::operator++() {
  *this += 1;
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer tmp = *this;
  ++*this;
  return tmp;
}

big_integer& big_integer::operator--() {
  *this -= 1;
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer tmp = *this;
  --*this;
  return tmp;
}

big_integer operator+(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out += b;
  return out;
}

big_integer operator-(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out -= b;
  return out;
}

big_integer operator*(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out *= b;
  return out;
}

big_integer operator/(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out /= b;
  return out;
}

big_integer operator%(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out %= b;
  return out;
}

big_integer operator&(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out &= b;
  return out;
}

big_integer operator|(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out |= b;
  return out;
}

big_integer operator^(const big_integer& a, const big_integer& b) {
  big_integer out = a;
  out ^= b;
  return out;
}

big_integer operator<<(const big_integer& a, int b) {
  big_integer out = a;
  out <<= b;
  return out;
}

big_integer operator>>(const big_integer& a, int b) {
  big_integer out = a;
  out >>= b;
  return out;
}

bool operator==(const big_integer& a, const big_integer& b) noexcept {
  return (a.isZero() && b.isZero()) || ((a.sign == b.sign) && a.data_ == b.data_);
}

bool operator!=(const big_integer& a, const big_integer& b) noexcept {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) noexcept {
  if (a.isZero() && b.isZero()) {
    return false;
  }
  if (a.sign != b.sign) {
    return a.sign < b.sign;
  }
  if (a == b) {
    return false;
  }
  if (a.data_.size() != b.data_.size()) {
    return (a.sign > 0) ^ (a.data_.size() > b.data_.size());
  }
  return (a.sign == 1) ^
         (!std::lexicographical_compare(a.data_.rbegin(), a.data_.rend(), b.data_.rbegin(), b.data_.rend()));
}

bool operator>(const big_integer& a, const big_integer& b) noexcept {
  return (b < a);
}

bool operator<=(const big_integer& a, const big_integer& b) noexcept {
  return !(a > b);
}

bool operator>=(const big_integer& a, const big_integer& b) noexcept {
  return !(a < b);
}

uint64_t big_integer::divModByConst(uint64_t rhs) {
  uint64_t carry = 0;
  for (size_t i = data_.size(); i > 0; --i) {
    uint64_t tmp = data_[i - 1] + carry * LIMIT;
    data_[i - 1] = (tmp / rhs) % LIMIT;
    carry = tmp % rhs;
  }
  deleteLeadingZeroes();
  return carry;
}

std::string to_string(const big_integer& a) {
  if (a.isZero()) {
    return "0";
  }
  std::string res;
  big_integer tmp = a;
  tmp.sign = 1;
  while (tmp > 0) {
    uint64_t num = tmp.divModByConst(a.STR_NUMS);
    std::string tmp_digits = std::to_string(num);
    for (std::string::reverse_iterator it = tmp_digits.rbegin(); it < tmp_digits.rend(); ++it) {
      res += *it;
    }
    if (tmp > 0) {
      res += std::string(a.STR_NUMS_COUNT - tmp_digits.size(), '0');
    }
  }
  if (a.sign == -1) {
    res += "-";
  }
  std::reverse(res.begin(), res.end());
  return res;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}
