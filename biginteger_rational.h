#include <cstring>
#include <istream>
#include <iostream>
#include <vector>
class BigInteger;
bool operator>(const BigInteger&, const BigInteger&);
bool operator>=(const BigInteger&, const BigInteger&);
bool operator<(const BigInteger&, const BigInteger&);
bool operator<=(const BigInteger&, const BigInteger&);
bool operator !=(const BigInteger&, const BigInteger&);
bool operator ==(const BigInteger&, const BigInteger&);
BigInteger abs(const BigInteger& bi);

std::istream& operator>>(std::istream &in, BigInteger& bi);
std::ostream& operator<<(std::ostream &out, const BigInteger& bi);

BigInteger operator+(const BigInteger&, const BigInteger&);
BigInteger operator-(const BigInteger&, const BigInteger&);
BigInteger operator *(const BigInteger&, const BigInteger&);
BigInteger operator /(const BigInteger&, const BigInteger&);
BigInteger operator %(const BigInteger&, const BigInteger&);

BigInteger operator ""_bi(const char*, size_t);
BigInteger operator ""_bi(unsigned long long);
class BigInteger {
  private:
  static const long long base = 1000000000;
  std::vector<int> digits_;
  bool is_positive_;

  void RemoveLeadingZeroes();
  void SumOfModules(BigInteger& first, BigInteger second);
  void DiffOfModules(BigInteger& first, BigInteger second);
  void DiffSupport(BigInteger& first, const BigInteger& second);

  BigInteger AddEndZeroes(size_t num);
  BigInteger substr(BigInteger& base, size_t start, size_t count);

 public:
  BigInteger() = default;
  BigInteger(int num);
  explicit BigInteger(unsigned long long);
  explicit operator bool() const;
  BigInteger(std::string str);
  BigInteger(const char* str);
  //BigInteger(const BigInteger& num);

  const std::vector<int>& data() const;
  std::vector<int>& data();
  std::string toString() const;
  std::string toString(int) const;
  bool isPositive() const;

  BigInteger operator -() const {
    BigInteger temp = *this;
    if (temp != 0) temp.is_positive_ = !temp.is_positive_;
    return temp;
  }
  BigInteger& operator++() {
    *this += 1;
    return *this;
  }
  BigInteger& operator--() {
    *this -= 1;
    return *this;
  }
  BigInteger operator++(int) {
    ++*this;
    return *this - 1;
  }
  BigInteger operator --(int) {
    --*this;
    return *this + 1;
  }
  BigInteger& operator +=(const BigInteger& other) {
    if ((is_positive_ and !other.is_positive_) or (!is_positive_ and other.is_positive_)) {
      DiffOfModules(*this, other);
      if (abs(*this) == 0)
        is_positive_ = true;
      return *this;
    }
    SumOfModules(*this, other);
    if (abs(*this) == 0)
      is_positive_ = true;
    return *this;
  }
  BigInteger& operator -=(const BigInteger& other) {
    return *this += -other;
  }
  BigInteger& operator *=(const BigInteger& other) {
    RemoveLeadingZeroes();
    BigInteger res;
    res.digits_.resize(size() + other.size(), 0);
    bool flag = ((is_positive_ && other.is_positive_) || (!is_positive_ && !other.is_positive_));
    is_positive_ = true;
    for (size_t i = 0; i < size(); ++i) {
      int remainder = 0;
      for (size_t j = 0; j < other.size(); ++j) {
        long long help = 1;
        help *= digits_[i];
        help *= other[j];
        help += remainder;
        help += res[i + j];
        res[i + j] = help % base;
        remainder = help / base;
      }
      if (remainder) res[i + other.size()] += remainder;
    }
    *this = res;
    RemoveLeadingZeroes();
    if (size() == 1 and digits_[0] == 0) is_positive_ = true;
    else is_positive_ = flag;
    return *this;
  }
  BigInteger& operator /=(const BigInteger& other) {
    RemoveLeadingZeroes();
    bool flag = ((is_positive_ && other.is_positive_) || (!is_positive_ && !other.is_positive_));
    is_positive_ = true;
    BigInteger remainder = 0, res;
    for (size_t i = size(); i > 0; --i) {
      remainder *= base;
      remainder += digits_[i - 1];
      if (remainder >= other) {
        int l = 0, r = base;
        while (r - l > 1) {
          int mid = (r + l) >> 1;
          if (mid * abs(other) <= remainder) l = mid;
          else r = mid;
        }
        remainder -= l * abs(other);
        res.digits_.emplace_back(l);
      } else res.digits_.emplace_back(0);
    }
    size_t sz = res.size();
    for (size_t i = 0; i < sz / 2; ++i) {
      std::swap(res[i], res[sz - i - 1]);
    }
    *this = res;
    RemoveLeadingZeroes();
    if (size() == 1 and digits_[0] == 0) is_positive_ = true;
    else is_positive_ = flag;
    return *this;
  }
  BigInteger& operator %=(const BigInteger& other) {
    *this -= (*this / other) * other;
    return *this;
  }
  size_t size() const {
    return digits_.size();
  }
  int& operator[](size_t i) {
    return digits_[i];
  }
  const int& operator[](size_t i) const {
    return digits_[i];
  }
};
BigInteger BigInteger::substr(BigInteger& basa, size_t start, size_t count) {
  BigInteger temp;
  temp.digits_.resize(count);
  for (size_t i = 0; i < count; ++i) {
    temp[i] = basa[basa.size() - start - i];
  }
  return temp;
}
void BigInteger::RemoveLeadingZeroes() {
  while (digits_.size() > 1 && digits_.back() == 0) digits_.pop_back();
}
const std::vector<int>& BigInteger::data() const {
  return digits_;
}
std::vector<int>& BigInteger::data(){
  return digits_;
}
void BigInteger::SumOfModules(BigInteger& first, BigInteger second) {
  std::vector<int> temp = first.digits_;
  if (second.size() > first.size()) {
    first.digits_.resize(std::max(first.size(), second.size()), 0);
    for (size_t i = 0; i < temp.size(); ++i) {
      first[i] = temp[i];
    }
  }
  int remainder = 0;
  for (size_t i = 0; i < first.size(); ++i) {
    int temp_rem = remainder;
    if (i < first.size()) temp_rem += first[i];
    if (i < second.size()) temp_rem += second[i];
    first[i] = temp_rem % base;
    remainder = temp_rem / base;
  }
  if (remainder != 0) first.digits_.emplace_back(remainder);
}
void BigInteger::DiffSupport(BigInteger& first, const BigInteger& second) {
  int remainder = 0;
  for (size_t i = 0; i < first.size(); ++i) {
    if (first[i] < (i < second.size() ? second[i] : 0) + remainder) {
      first[i] += base - (i < second.size() ? second[i] : 0) - remainder;
      remainder = 1;
    } else {
      first[i] -= (i < second.size() ? second[i] : 0) + remainder;
      remainder = 0;
    }
  }
  first.RemoveLeadingZeroes();
}
void BigInteger::DiffOfModules(BigInteger& first, BigInteger second){
  if (abs(second) > abs(first)) {
    DiffSupport(second, first);
    first = second;
  } else {
    DiffSupport(first, second);
  }
}
BigInteger::BigInteger(unsigned long long num) {
  while (num > 0) {
    digits_.emplace_back(num % base);
    num /= base;
    if (num) digits_.emplace_back(num);
  }
}
BigInteger::operator bool() const {
  if (*this != 0) return true;
  return false;
}
BigInteger::BigInteger(int num): is_positive_(num >= 0) {
  if (num < 0) num = -num;
  digits_.emplace_back(num % base);
  num /= base;
  if (num) digits_.emplace_back(num);
}
BigInteger::BigInteger(std::string str) {
  is_positive_ = (str[0] != '-');
  if (is_positive_) {
    int i;
    for (i = str.size() - 9; i >= 0; i -= 9) {
      digits_.emplace_back(stoi(str.substr(i, 9)));
    }
    if (i + 9 > 0) digits_.emplace_back(stoi(str.substr(0, (i + 9 < static_cast<int>(str.size()) ? i + 9 : str.size()))));
  } else {
    int i;
    for (i = str.size() - 9; i >= 1; i -= 9) {
      digits_.emplace_back(stoi(str.substr(i, 9)));
    }
    if (i + 8 > 0) digits_.emplace_back(stoi(str.substr(1, (i + 8 < static_cast<int>(str.size()) - 1? i + 8 : str.size() - 1))));
  }
}
BigInteger::BigInteger(const char* str) {
  std::string str1 = str;
  *this = str1;
}
std::string BigInteger::toString() const {
  std::string str = "";
  if (!is_positive_) str = "-";
  for (size_t i = 0; i < size(); ++i) {
    std::string temp = std::to_string(digits_[size() - i - 1]);
    while (temp.size() != 9 && i != 0) {
      temp = "0" + temp;
    }
    str += temp;
  }
  return str;
}
std::string BigInteger::toString(int) const {
  std::string str;
  for (size_t i = 0; i < size(); ++i) {
    std::string temp = std::to_string(digits_[size() - i - 1]);
    while (temp.size() != 9 && i != 0) {
      temp = "0" + temp;
    }
    str += temp;
  }
  return str;
}
bool BigInteger::isPositive() const {
  return is_positive_;
}
BigInteger operator ""_bi(const char* bi, size_t) {
  std::string str = bi;
  BigInteger temp = str;
  return temp;
}
BigInteger operator ""_bi(unsigned long long num) {
  BigInteger temp = num;
  return temp;
}
BigInteger operator +(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res += b;
  return res;
}
BigInteger operator -(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res -= b;
  return res;
}
BigInteger operator *(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res *= b;
  return res;
}
BigInteger operator /(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res /= b;
  return res;
}
BigInteger operator %(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res %= b;
  return res;
}
bool operator<(const BigInteger& a, const BigInteger& b) {
  if ((a.isPositive() && !b.isPositive()) or (!a.isPositive() && b.isPositive())) return b.isPositive();
  if (a.size() > b.size()) return !a.isPositive();
  if (a.size() < b.size()) return a.isPositive();
  for (size_t i = 0; i < a.size(); ++i) {
    if (a.data()[a.size() - i - 1] < b.data()[a.size() - i - 1]) return a.isPositive();
    if (a.data()[a.size() - i - 1] > b.data()[a.size() - i - 1]) return !a.isPositive();
  }
  return false;
}
bool operator ==(const BigInteger& a, const BigInteger& b) {
  if (a.size() != b.size() || a.isPositive() != b.isPositive()) return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i] != b[i]) return false;
  return true;
}
bool operator >(const BigInteger& a, const BigInteger& b) {
  return b < a;
}
bool operator !=(const BigInteger& a, const BigInteger& b) {
  return !(a == b);
}
bool operator >=(const BigInteger& a, const BigInteger& b) {
  return !(a < b);
}
bool operator <=(const BigInteger& a, const BigInteger& b) {
  return !(b < a);
}
std::ostream& operator<<(std::ostream &out, const BigInteger& bi) {
  std::string str = bi.toString();
  out << str;
  return out;
}
std::istream& operator>>(std::istream &in, BigInteger& bi) {
  std::string str;
  in >> str;
  bi = str;
  return in;
}
class Rational {
  private:
  BigInteger numerator_;
  BigInteger denominator_;
  bool is_positive_;

  void simplifyFraction();
  public:
  friend bool operator <(const Rational&, const Rational&);
  friend bool operator ==(const Rational&, const Rational&);

  Rational() {}
  Rational(BigInteger bi);
  Rational (int num);
  bool isPositive() const;
  std::string toString() const;

  std::string asDecimal(size_t precision) const;

  Rational operator -();
  explicit operator double() const;
  Rational& operator +=(const Rational& other);
  Rational& operator -=(const Rational& other);
  Rational& operator *=(const Rational& other);
  Rational& operator /=(const Rational& other);
};
BigInteger abs(const BigInteger& bi) {
  return (bi.isPositive() ? bi : -bi);
}
BigInteger greatestCD(BigInteger first, BigInteger second) {
  if (!first.isPositive()) first = -first;
  if (!second.isPositive()) second = -second;
  if (first == 0) return second;
  if (second == 0) return first;
  if (first < second) return greatestCD(first, second % first);
  return greatestCD(first % second, second);
}
void Rational::simplifyFraction() {
  BigInteger gcd = greatestCD(numerator_, denominator_);
  numerator_ /= gcd;
  denominator_ /= gcd;
}
bool Rational::isPositive() const {
  return is_positive_;
}
std::string Rational::toString() const {
  std::string str = (is_positive_ ? "" : "-");
  str += abs(numerator_).toString();
  if (denominator_ != 1 && denominator_ != -1) {
    str += "/";
    str += abs(denominator_).toString();
  }
  return str;
}
std::string Rational::asDecimal(size_t precision) const {
  std::string multiplier(precision, '0');
  multiplier = "1" + multiplier;
  BigInteger temp = numerator_;
  temp *= multiplier;
  std::string res = (temp / denominator_).toString(0);
  if (res.size() <= precision) {
    std::string str(precision + 1 - res.size(), '0');
    if (isPositive()) res = str + res;
    else res = '-' + str + res;
  }
  res.insert(res.end() - precision, '.');
  return res;
}
Rational::Rational(BigInteger bi): numerator_(bi), denominator_(1), is_positive_(bi.isPositive()) {}
Rational::Rational(int num): numerator_(num), denominator_(1), is_positive_(num >= 0) {}
Rational Rational::operator -() {
  Rational temp = *this;
  if (numerator_ != 0) temp.is_positive_ = !is_positive_;
  return temp;
}
Rational::operator double() const {
  double dbl = std::stod(asDecimal(15));
  return dbl;
}
Rational& Rational::operator +=(const Rational& other) {
  if (denominator_ == other.denominator_) {
    numerator_ += other.numerator_;
    simplifyFraction();
    return *this;
  }
  numerator_ *= other.denominator_;
  numerator_ += denominator_ * other.numerator_;
  denominator_ *= other.denominator_;
  simplifyFraction();
  return *this;
}
Rational& Rational::operator -=(const Rational& other) {
  if (denominator_ == other.denominator_) {
    numerator_ -= other.numerator_;
    simplifyFraction();
    is_positive_ = ((numerator_.isPositive() && denominator_.isPositive()) || (!numerator_.isPositive() && !denominator_.isPositive()));
    return *this;
  }
  numerator_ *= other.denominator_;
  numerator_ -= denominator_ * other.numerator_;
  denominator_ *= other.denominator_;
  simplifyFraction();
  is_positive_ = ((numerator_.isPositive() && denominator_.isPositive()) || (!numerator_.isPositive() && !denominator_.isPositive()));
  return *this;
}
Rational& Rational::operator *=(const Rational& other) {
  if (numerator_ == 0 or other.numerator_ == 0) {
    *this = 0;
    return *this;
  }
  numerator_ *= other.numerator_;
  denominator_ *= other.denominator_;
  simplifyFraction();
  is_positive_ = ((isPositive() && other.isPositive()) || (!isPositive() && !other.isPositive()));
  return *this;
}
Rational& Rational::operator /=(const Rational& other) {
  if (numerator_ == 0 or other.numerator_ == 0) {
    *this = 0;
    return *this;
  }
  numerator_ *= other.denominator_;
  denominator_ *= other.numerator_;
  simplifyFraction();
  is_positive_ = ((isPositive() && other.isPositive()) || (!isPositive() && !other.isPositive()));
  return *this;
}
Rational operator *(Rational a, const Rational& b) {
  return a *= b;
}
Rational operator /(Rational a, const Rational& b) {
  return a /= b;
}
Rational operator +(Rational a, const Rational& b) {
  return a += b;
}
Rational operator -(Rational a, const Rational& b) {
  return a -= b;
}
bool operator<(const Rational& a, const Rational& b) {
  if (a.isPositive() && !b.isPositive()) return false;
  if (!a.isPositive() && b.isPositive()) return true;
  return (abs(a.numerator_ * b.denominator_) < abs(a.denominator_ * b.numerator_));
}
bool operator ==(const Rational& a, const Rational& b) {
  return ((abs(a.numerator_) == abs(b.numerator_) && abs(a.denominator_) == abs(b.denominator_)) and a.isPositive() == b.isPositive());
}
bool operator >(const Rational& a, const Rational& b) {
  return b < a;
}
bool operator !=(const Rational& a, const Rational& b) {
  return !(a == b);
}
bool operator >=(const Rational& a, const Rational& b) {
  return !(a < b);
}
bool operator <=(const Rational& a, const Rational& b) {
  return !(b < a);
}
