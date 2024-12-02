#include <Eigen/Dense>

export module Shapes;

import std;

export struct Point {
    double x, y, z;

    // 생성자
    Point() : x(0), y(0), z(0) {}
    Point(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Point(std::initializer_list<double> init_list) {
        auto it = init_list.begin();
        x = (it != init_list.end()) ? *it++ : 0.0;
        y = (it != init_list.end()) ? *it++ : 0.0;
        z = (it != init_list.end()) ? *it++ : 0.0;
    }

    // 비교 연산자
    bool operator==(const Point& other) const {
        constexpr double epsilon = 1e-6;
        return std::abs(x - other.x) < epsilon &&
            std::abs(y - other.y) < epsilon &&
            std::abs(z - other.z) < epsilon;
    }

    // 두 점 사이의 거리 계산
    double distance(const Point& other) const {
        return std::sqrt((x - other.x) * (x - other.x) +
            (y - other.y) * (y - other.y) +
            (z - other.z) * (z - other.z));
    }

    // Eigen Vector3d로 변환
    Eigen::Vector3d toVector() const {
        return Eigen::Vector3d(x, y, z);
    }

    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return z < other.z;
    }

    bool operator>(const Point& other) const {
        return other < *this; // Use the existing < operator
    }
};

export auto pointHash = [](const Point& p) -> std::size_t
    {
        std::size_t hx = std::hash<double>()(p.x);
        std::size_t hy = std::hash<double>()(p.y);
        std::size_t hz = std::hash<double>()(p.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    };

export struct Triangle {
    Point p1, p2, p3;

    bool operator==(const Triangle& other) const {
        std::vector<Point> points1 = { p1, p2, p3 };
        std::vector<Point> points2 = { other.p1, other.p2, other.p3 };
        std::sort(points1.begin(), points1.end(), [](const Point& a, const Point& b) {
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            return a.z < b.z;
            });
        std::sort(points2.begin(), points2.end(), [](const Point& a, const Point& b) {
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            return a.z < b.z;
            });
        return points1 == points2;
    }

    double getArea() const {
        Eigen::Vector3d v0 = p1.toVector();
        Eigen::Vector3d v1 = p2.toVector();
        Eigen::Vector3d v2 = p3.toVector();
        Eigen::Vector3d crossProduct = (v1 - v0).cross(v2 - v0);
        return 0.5 * crossProduct.norm();
    }

    bool containsPoint(const Point& p) const {
        double totalArea = getArea();
        double subArea1 = Triangle{ p, p2, p3 }.getArea();
        double subArea2 = Triangle{ p1, p, p3 }.getArea();
        double subArea3 = Triangle{ p1, p2, p }.getArea();
        double sum = subArea1 + subArea2 + subArea3;
        return std::abs(totalArea - sum) < 1e-6;
    }

    Eigen::Vector3d getNormal() const {
        Eigen::Vector3d v0 = p1.toVector();
        Eigen::Vector3d v1 = p2.toVector();
        Eigen::Vector3d v2 = p3.toVector();
        Eigen::Vector3d normal = (v1 - v0).cross(v2 - v0);
        return normal.normalized();
    }
};

export struct Circle
{
    double radius;
    Point center;

    bool inCircle(Point p)
    {
        double dx = p.x - center.x;
        double dz = p.z - center.z;

        double distance = std::sqrt(dx * dx + /*dy * dy +*/ dz * dz);
        return distance < radius;
    }
};

export struct Edge
{
    Point p1;
    Point p2;

    bool operator==(const Edge& other) const
    {
        return (p1 == other.p1 && p2 == other.p2) || (p1 == other.p2 && p2 == other.p1);
    }
};

export Circle makeCircumcircle(Triangle t)
{
    Point p1 = t.p1;
    Point p2 = t.p2;
    Point p3 = t.p3;

    double ax = p1.x, az = p1.z;
    double bx = p2.x, bz = p2.z;
    double cx = p3.x, cz = p3.z;

    double d = 2 * (ax * (bz - cz) + bx * (cz - az) + cx * (az - bz));
    if (d == 0) return { 0, {0,0,0} };

    double x = ((ax * ax + az * az) * (bz - cz) + (bx * bx + bz * bz) * (cz - az) + (cx * cx + cz * cz) * (az - bz)) / d;
    double z = ((ax * ax + az * az) * (cx - bx) + (bx * bx + bz * bz) * (ax - cx) + (cx * cx + cz * cz) * (bx - ax)) / d;

    Point center = { x, 0, z };
    double radius = std::sqrt((center.x - ax) * (center.x - ax) + (center.z - az) * (center.z - az));

    return { radius, {center.x, 0, center.z} };
}

export struct Vector
{
    double compX = 0;
    double compY = 0;
    double compZ = 0;

    Vector(double inputX, double inputY, double inputZ) : compX(inputX), compY(inputY), compZ(inputZ)
    {
    }
};


