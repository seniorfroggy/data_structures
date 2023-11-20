#include <vector>
#include <iostream>
#include <cmath>
namespace constants {
  double epsilon = 1e-9;
  double pi_degree = 180;
  double pi = 3.14159265358979323846;
}
bool isEqual(double x, double y) {
  return fabs(x - y) < constants::epsilon;
}
class Point{
 public:
  double x;
  double y;
  Point(): x(0.0), y(0.0) {};
  Point(double x, double y): x(x), y(y) {};
};
bool operator==(const Point& a, const Point& b) {
  return (isEqual(a.x, b.x) and isEqual(a.y, b.y));
}
bool operator!=(const Point& a, const Point& b) {
  return !(a == b);
}
std::ostream& operator<<(std::ostream& os, Point a) {
  os << a.x << ' ' << a.y << '\n';
  return os;
}
Point segment_center(const Point& a,const Point& b) {
  Point temp((a.x + b.x) / 2, (a.y + b.y) / 2);
  return temp;
}
class Line  {
 public:
  double A, B, C; // Ax + By + C = 0
  Line();
  Line(Point, Point);
  Line(Point, double);
  Line(double, double);
};
bool operator==(const Line& a, const Line& b) {
  return isEqual(a.A, b.A) and isEqual(a.B, b.B) and isEqual(a.C, b.C);
}
bool operator!=(const Line& a, const Line& b) {
  return !(a == b);
}
Line::Line(): A(0), B(0), C(0){
}
Line::Line(Point a, Point b) {
  if (isEqual(a.x,  b.x)) {
    A = 1;
    B = 0;
    C = -a.x;
  } else if (isEqual(a.y, b.y)) {
    A = 0;
    B = 1;
    C = -a.y;
  } else {
    A = (a.y - b.y) / (a.x - b.x);
    B = -1;
    C = (a.x * b.y - b.x * a.y) / (a.x - b.x);
  }
}
Line::Line(Point point, double A) : A(A), B(-1), C(point.y - A * point.x) {
}
Line::Line(double A, double C) : A(A), B(-1), C(C) {}
std::ostream& operator<<(std::ostream& os,const Line& a) {
  os << a.A << "x + " << a.B << "y + "<< a.C << std::endl;
  return os;
}
Point intersectionPoint(const Line& a,const Line& b) {
  double x, y;
  if (isEqual(a.B, 0.0)) {
    x = -a.C / a.A;
    y = (-b.C - b.A * x) / b.B;
  } else if (isEqual(a.A, 0.0)) {
    y = -a.C / a.B;
    x = (-b.C - b.B * y) / b.A;
  } else {
    x = (a.B * b.C - b.B * a.C) / (b.B * a.A - b.A * a.B);
    y = (-a.C - a.A * x) / a.B;
  }
  if (isEqual(x, 0.0)) x = fabs(x);
  if (isEqual(y, 0.0)) y = fabs(y);
  Point temp(x, y);
  return temp;
}
Point symmetrical_about_point(const Point& var,const Point& base) {
  Point temp(2 * base.x - var.x, 2 * base.y - var.y);
  return temp;
}
Point symmetrical_about_line(const Point& var,const Line& base) {
  Line orth_base;
  orth_base.A = -base.B;
  orth_base.B = base.A;
  orth_base.C = base.B * var.x - base.A * var.y;
  return symmetrical_about_point(var, intersectionPoint(base, orth_base));
}
Point rotated_about_point(const Point& var,const Point& base, double angle) {
  double x, y;
  x = ((var.x - base.x) * cos(angle * constants::pi / constants::pi_degree) -
       (var.y - base.y) * sin(angle * constants::pi / constants::pi_degree) + base.x);
  y = (var.x - base.x) * sin(angle * constants::pi / constants::pi_degree) +
      (var.y - base.y) * cos(angle * constants::pi / constants::pi_degree) + base.y;
  Point temp(x, y);
  return temp;
}
Point scaledPoint(const Point& var, const Point& base, double coefficient) {
  double x, y;
  x = base.x + coefficient * (var.x - base.x);
  y = base.y + coefficient * (var.y - base.y);
  Point temp(x, y);
  return temp;
}

class Shape {
 public:
  virtual void rotate(const Point& center, double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;
  virtual void scale(const Point& center, double coefficient) = 0;
  virtual double perimeter() const = 0;
  virtual ~Shape() = default;
  virtual bool operator==(const Shape& another) const = 0;
  virtual bool isCongruentTo(const Shape& another) const = 0;
  virtual bool isSimilarTo(const Shape& another) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;
  virtual double area() const = 0;
};
double dist(const Point& a,const Point& b) {
  return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
class Polygon : public Shape {
 protected:
  std::vector<Point> points;
  void construct(const Point& point) { points.emplace_back(point); }
  void construct() {}
 public:
  explicit Polygon(std::vector<Point>& points) : points(points) {};
  template<typename Front, typename... Args>
  void construct(const Front& front,const Args&... args) {
    points.emplace_back(front);
    construct(args...);
  }
  template<typename... Args>
  explicit Polygon(const Args... args) {
    construct(args...);
  }
  Polygon();

  bool operator==(const Polygon& another) const;
  bool operator==(const Shape& another) const override;
  bool isCongruentTo(const Shape& another) const override;
  bool isSimilarTo(const Shape& another) const override;
  bool containsPoint(const Point& point) const override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;
  void scale(const Point& center, double coefficient) override;
  double perimeter() const override;
  double area() const override;
  size_t verticesCount() const;
  const std::vector<Point>& getVertices() const;
  bool isConvex() const;
  ~Polygon() = default;
};
Polygon::Polygon() {
  points = {};
}
bool Polygon::operator==(const Polygon& another) const {
  if (another.points.size() != points.size()) {
    return false;
  }
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    bool flag = true;
    for (size_t j = 0; j < size; ++j) {
      if (points[(j + i) % size] != (another.points[j])) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
    flag = true;
    for (size_t j = 0; j < size; ++j) {
      if (points[(j + i) % size] != (another.points[size - j - 1])) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
  }
  return false;
}
bool Polygon::isSimilarTo(const Shape &another) const {
  auto& other = const_cast<Shape&>(another);
  if (dynamic_cast<Polygon*>(&other) == nullptr) {
    return false;
  }
  auto temp = dynamic_cast<Polygon*>(&other);
  if (temp->points.size() != points.size()) {
    return false;
  }
  double areas_ratio = area() / temp->area();
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    bool flag = true;
    for (size_t j = 0; j < size; ++j) {
      double first_dist = dist(points[(j + i) % size], points[(j + i + 1) % size]);
      double second_dist = dist(temp->points[j], temp->points[(j + 1) % size]);
      if (!isEqual(first_dist / second_dist,sqrt(areas_ratio))) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
    flag = true;
    for (size_t j = 0; j < size; ++j) {
      double first_dist = dist(points[(j + i) % size], points[(j + i + 1) % size]);
      double second_dist = dist(temp->points[size - j - 1], temp->points[(size - j) % size]);
      if (!isEqual(first_dist / second_dist,sqrt(areas_ratio))) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
  }
  return false;
}
bool Polygon::isCongruentTo(const Shape& another) const {
  return (isEqual(area(), another.area()) and isSimilarTo(another));
}
bool Polygon::operator==(const Shape& another) const {
  auto& other = const_cast<Shape&>(another);
  if (dynamic_cast<Polygon*>(&other) == nullptr) {
    return false;
  }
  auto temp = dynamic_cast<Polygon*>(&other);
  if (temp->points.size() != points.size()) {
    return false;
  }
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    bool flag = true;
    for (size_t j = 0; j < size; ++j) {
      if (points[(j + i) % size] != (temp->points[j])) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
    flag = true;
    for (size_t j = 0; j < size; ++j) {
      if (points[(j + i) % size] != (temp->points[size - j - 1])) {
        flag = false;
        break;
      }
    }
    if (flag)
      return true;
  }
  return false;
}
bool Polygon::containsPoint(const Point& point) const { //PNPoly algo
  bool flag = false;
  size_t size = points.size(), j = size - 1;
  for (size_t i = 0; i < size; ++i) {
    if (((points[i].y > point.y) != (points[j].y > point.y)) and
        (point.x < (points[j].x - points[i].x) * (point.y - points[i].y) /
        (points[j].y - points[i].y) + points[i].x)) {
      flag = !flag;
    }
    j = i;
  }
  return flag;
}
void Polygon::scale(const Point &center, double coefficient) {
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    points[i] = scaledPoint(points[i], center, coefficient);
  }
}
void Polygon::reflect(const Point &center) {
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    points[i] = symmetrical_about_point(points[i], center);
  }
}
void Polygon::reflect(const Line &axis) {
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    points[i] = symmetrical_about_line(points[i], axis);
  }
}
void Polygon::rotate(const Point &center, double angle) {
  size_t size = points.size();
  for (size_t i = 0; i < size; ++i) {
    points[i] = rotated_about_point(points[i], center, angle);
  }
}
double Polygon::perimeter() const {
  double sum = 0;
  for (size_t i = 0; i < verticesCount() - 1; ++i) {
    sum += dist(points[i], points[i + 1]);
  }
  sum += dist(points[0], points[verticesCount() - 1]);
  return sum;
}
double Polygon::area() const {
  double sum = 0;
  size_t size = verticesCount();
  for (size_t i = 0; i < size; i++) {
    sum += (points[i].x - points[(i + 1) % size].x) *
           (points[i].y + points[(i + 1) % size].y);
  }
  return fabs(sum) / 2;
}
const std::vector<Point>& Polygon::getVertices() const {
  return points;
}
size_t Polygon::verticesCount() const {
  return points.size();
}
bool Polygon::isConvex() const {
  size_t pos = 0, size = verticesCount();
  for (size_t i = 0; i < size; ++i) {
    double dx1 = (points[(i + 1) % size].x - points[i % size].x),
           dx2 = points[(i + 2) % size].x - points[(i + 1) % size].x,
           dy1 = (points[(i + 1) % size].y - points[i % size].y),
           dy2 = points[(i + 2) % size].y - points[(i + 1) % size].y;
    double z_cross = dx1 * dy2 - dy1 * dx2;
    pos += (z_cross > 0 ? 1 : 0);
  }
  return (pos == 0 or pos == size);
}

class Ellipse : public Shape {
 protected:
  Point focus1;
  Point focus2;
  double a;
  double b;
  double c;
  Line directrix1;
  Line directrix2;
 public:
  Ellipse(Point, Point, double);
  Ellipse();
  bool operator==(const Shape& another) const override;
  bool operator==(const Ellipse& another) const;
  bool isCongruentTo(const Shape& another) const override;
  bool isSimilarTo(const Shape& another) const override;
  bool containsPoint(const Point& point) const override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;
  void scale(const Point& center, double coefficient) override;
  double perimeter() const override;
  double area() const override;
  std::pair<Point, Point> focuses() const;
  std::pair<Line, Line> directrices() const ;
  double eccentricity() const;
  Point center() const;
};
Ellipse::Ellipse(): a(0), b(0), c(0){}
Ellipse::Ellipse(Point first, Point second, double distance) {
  focus1 = first;
  focus2 = second;
  c = dist(focus1, center());
  a = distance / 2;
  b = sqrt(a * a - c * c);
  Line axis(focus1, focus2), orth_axis;
  Point inter = scaledPoint(focus1, center(), a * a / (c * c));
  orth_axis.A = -axis.B;
  orth_axis.B = axis.A;
  orth_axis.C = axis.B * inter.x - axis.A * inter.y;
  directrix1 = orth_axis;
  inter = symmetrical_about_point(inter, center());
  orth_axis.C = axis.B * inter.x - axis.A * inter.y;
  directrix2 = orth_axis;
}
bool Ellipse::operator==(const Ellipse& another) const {
  if (another.focus1 == focus1) {
    return (another.focus2 == focus2 and isEqual(another.a, a));
  }
  return (another.focus2 == focus1 and isEqual(another.a, a));
}
bool Ellipse::isCongruentTo(const Shape &another) const {
  auto& other = const_cast<Shape&>(another);
  if (dynamic_cast<Ellipse*>(&other) == nullptr) {
    return false;
  }
  auto temp = dynamic_cast<Ellipse*>(&other);
  return (isEqual(temp->a, a) and (isEqual(b, (temp->a * temp->a - temp->c * temp->c))));
}
bool Ellipse::isSimilarTo(const Shape &another) const {
  auto& other = const_cast<Shape&>(another);
  if (dynamic_cast<Ellipse*>(&other) == nullptr) {
    return false;
  }
  auto temp = dynamic_cast<Ellipse*>(&other);
  return isEqual(temp->eccentricity(), eccentricity());
}
bool Ellipse::containsPoint(const Point& point) const {
  return (dist(point, focus1) + dist(point, focus2) <= 2 * a);
}
bool Ellipse::operator==(const Shape& another) const {
  auto& other = const_cast<Shape&>(another);
  if (dynamic_cast<Ellipse*>(&other) == nullptr) {
    return false;
  }
  auto temp = dynamic_cast<Ellipse*>(&other);
  if (temp->focus1 == focus1) {
    return (temp->focus2 == focus2 and isEqual(temp->a, a));
  }
  return (temp->focus2 == focus1 and isEqual(temp->a, a));
}
void Ellipse::scale(const Point &center, double coefficient) {
  focus1 = scaledPoint(focus1, center, coefficient);
  focus2 = scaledPoint(focus2, center, coefficient);
  a *= coefficient;
  c *= coefficient;
}
void Ellipse::reflect(const Line &axis) {
  focus1 = symmetrical_about_line(focus1, axis);
  focus2 = symmetrical_about_line(focus2, axis);
}
void Ellipse::reflect(const Point &center) {
  focus1 = symmetrical_about_point(focus1, center);
  focus2 = symmetrical_about_point(focus2, center);
}
void Ellipse::rotate(const Point &center, double angle) {
  focus1 = rotated_about_point(focus1, center, angle);
  focus2 = rotated_about_point(focus2, center, angle);
}
double Ellipse::area() const {
  return b * a * constants::pi;
}
double Ellipse::perimeter() const {
  return constants::pi * (3 * (a + b) - sqrt((3 * a + b) * (3 * b + a)));
}
double Ellipse::eccentricity() const {
  return c / a;
}
Point Ellipse::center() const {
  Point temp = segment_center(focus1, focus2);
  return temp;
}
std::pair<Point, Point> Ellipse::focuses() const {
  return {focus1, focus2};
}
std::pair<Line, Line> Ellipse::directrices() const {
  return {directrix1, directrix2};
}

class Circle: public Ellipse{
 public:
  Circle(Point, double);
  double radius() const;
};
Circle::Circle(Point center, double radius): Ellipse(center, center, 2 * radius) {}
double Circle::radius() const {
  return a;
}
class Rectangle: public Polygon {
 public:
  Point center() const;
  std::pair<Line, Line> diagonals() const;
  Rectangle(Point, Point, double);
  Rectangle(Point, Point, Point, Point);
};

Rectangle::Rectangle(Point a, Point b, double ratio): Polygon() {
  points.resize(4);
  points[0] = a;
  points[2] = b;
  ratio = (ratio > 1 ? ratio : 1 / ratio);
  double smaller_side, bigger_side, diag = dist(a, b);
  smaller_side = sqrt(diag * diag / (ratio * ratio + 1));
  bigger_side = smaller_side * ratio;
  Point center = segment_center(points[0], points[2]), temp;
  temp.x = (b.x - a.x) * smaller_side / diag - (b.y - a.y) * bigger_side / diag + a.x;
  temp.y = (b.x - a.x) * bigger_side / diag + (b.y - a.y) * smaller_side / diag + a.y;
  temp = scaledPoint(temp, a, smaller_side / dist(a, temp));
  points[1] = temp;
  points[3] = symmetrical_about_point(temp, center);
}
Rectangle::Rectangle(Point a, Point b, Point c, Point d) {
  std::vector<Point> temp = {a, b, c, d};
  points = temp;
}
Point Rectangle::center() const {
  Point temp = segment_center(points[0], points[2]);
  return temp;
}
std::pair<Line, Line> Rectangle::diagonals() const {
  Line first(points[0], points[2]), second(points[1], points[3]);
  return {first, second};
}

class Square: public Rectangle {
 public:
  using Rectangle::Rectangle;
  Circle circumscribedCircle() const;
  Circle inscribedCircle() const;
  Square(Point a, Point b): Rectangle(a, b, 1) {};
};

Circle Square::circumscribedCircle() const {
  Circle temp(center(), sqrt(2) * dist(points[0], points[1]) / 2);
  return temp;
}
Circle Square::inscribedCircle() const {
  Circle temp(center(), dist(points[0], points[1]) / 2);
  return temp;
}

class Triangle: public Polygon {
 public:
  using Polygon::Polygon;
  Circle inscribedCircle() const;
  Circle circumscribedCircle() const;
  Point centroid() const;
  Point orthocenter() const;
  Line EulerLine() const;
  Circle ninePointsCircle() const;
};
Circle Triangle::inscribedCircle() const {
  double a, b, c;
  a = dist(points[1], points[2]);
  c = dist(points[0], points[2]);
  b = dist(points[0], points[1]);
  Point center((a * points[0].x + b * points[2].x + c * points[1].x) / (a + b + c),
               (a * points[0].y + b * points[2].y + c * points[1].y) / (a + b + c));
  Circle temp(center, 2 * area() / (a + b + c));
  return temp;
}
Circle Triangle::circumscribedCircle() const {
  double a, b, c;
  a = dist(points[1], points[2]);
  c = dist(points[0], points[2]);
  b = dist(points[0], points[1]);
  double x1 = points[0].x, x2 = points[1].x, x3 = points[2].x,
         y1 = points[0].y, y2 = points[1].y, y3 = points[2].y;
  double x12 = x1 - x2, x23 = x2 - x3, x31 = x3 - x1,
         y12 = y1 - y2, y23 = y2 - y3, y31 = y3 - y1,
         z1 = x1 * x1 + y1 * y1, z2 = x2 * x2 + y2 * y2,
         z3 = x3 * x3 + y3 * y3;
  Point center((-(y12 * z3 + y23 * z1 + y31 * z2) / (2 * (x12 * y31 - y12 * x31))),
               (x12 * z3 + x23 * z1 + x31 * z2) / (2 * (x12 * y31 - y12 * x31)));
  Circle temp(center, a * b * c / (4 * area()));
  return temp;
}
Point Triangle::centroid() const {
  Point A = points[0], B = points[1], C = points[2];
  Point temp(((A.x + B.x + C.x) / 3), (A.y + B.y + C.y) / 3);
  return temp;
}
Point Triangle::orthocenter() const {
  Point A = points[0], B = points[1], C = points[2];
  double x32 = C.x - B.x, y32 = C.y - B.y, z1 = A.x * x32 + A.y * y32,
         x31 = C.x - A.x, y31 = C.y - A.y, z2 = B.x * x31 + B.y * y31;
  double det = x32 * y31 - x31 * y32, det_x = z1 * y31 - z2 * y32, det_y = x32 * z2 - x31 * z1;
  Point temp(det_x / det, det_y / det);
  return temp;
}
Line Triangle::EulerLine() const {
  Line temp(circumscribedCircle().center(), orthocenter());
  return temp;
}
Circle Triangle::ninePointsCircle() const {
  Point center = segment_center(orthocenter(), circumscribedCircle().center());
  Circle temp(center, circumscribedCircle().radius() / 2);
  return temp;
}

