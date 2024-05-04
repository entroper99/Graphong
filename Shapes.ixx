export module Shapes;

import std;

export struct Point
{
    double x, y, z;

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    double distance(const Point& other) const
    {
        double dx = x - other.x;
        double dy = y - other.y;
        double dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

export struct Triangle
{
    Point p1, p2, p3;

    bool operator==(const Triangle& other) const
    {
        return (p1 == other.p1 || p1 == other.p2 || p1 == other.p3) &&
            (p2 == other.p1 || p2 == other.p2 || p2 == other.p3) &&
            (p3 == other.p1 || p3 == other.p2 || p3 == other.p3);
    }

    double getArea() const
    {
        double a = p1.distance(p2);
        double b = p2.distance(p3);
        double c = p3.distance(p1);
        double s = (a + b + c) / 2.0;
        return std::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    bool containsPoint(const Point& p) const
    {
        double totalArea = getArea();
        double subArea1 = Triangle{ p, p2, p3 }.getArea();
        double subArea2 = Triangle{ p1, p, p3 }.getArea();
        double subArea3 = Triangle{ p1, p2, p }.getArea();
        double sum = subArea1 + subArea2 + subArea3;
        return std::abs(totalArea - sum) < 1e-6;
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
    double radius = sqrt((center.x - ax) * (center.x - ax) + (center.z - az) * (center.z - az));

    return { radius, {center.x, 0, center.z} };
}