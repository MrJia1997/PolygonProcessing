#include "polygon.h"
#include <QtMath>


// Dot product
int Vector::operator*(Vector v) {
    return x * v.x + y * v.y;
}

// Cross product
int Vector::operator^(Vector v) {
    return x * v.y - y * v.x;
}

double Vector::module() {
    return qSqrt(x * x + y * y);
}

void Vector::rotate(double sinB, double cosB) {
    int prevX = x, prevY = y;
    x = static_cast<int>(prevX * cosB - prevY * sinB);
    y = static_cast<int>(prevX * sinB + prevY * cosB);
    return;
}

Point Point::operator+(Vector v) {
    return Point(x + v.x, y + v.y);
}

Vector Point::operator-(Point p) {
    return Vector(x - p.x, y - p.y);
}

void Point::translate(Vector delta) {
    x += delta.x;
    y += delta.y;
    return;
}

void Point::translate(int deltaX, int deltaY) {
    x += deltaX;
    y += deltaY;
    return;
}

int SimplePolygon::isClockwise() {

    if (vertices.size() < 3)
        return -1; // No enough vertices

    // Find the left most vertex
    int v = vertices.size();
    int min_x = INT_MAX, min_index = 0;
    for (int i = 0; i < v; i++) {
        if (vertices[i].x < min_x) {
            min_x = vertices[i].x;
            min_index = i;
        }
    }

    //Calculate cross product of the two adjacent vectors
    int prev = (min_index - 1 + v) % v, next = (min_index + 1) % v;
    Vector v1 = vertices[min_index] - vertices[prev];
    Vector v2 = vertices[next] - vertices[min_index];
    int xp = v1 ^ v2;

    if (xp < 0)
        return CLOCKWISE;
    else if (xp > 0)
        return COUNTERCLOCKWISE;
    else
    {
        if (vertices[prev].y < vertices[next].y)
            return CLOCKWISE;
        else
            return COUNTERCLOCKWISE;
    }

}

void SimplePolygon::reverseVertices() {
    int n = vertices.size();
    for (int i = 0; i < n / 2; i++)
        vertices.swap(i, n - i - 1);
}

bool SimplePolygon::isInsideSimplePolygon(Point p) {
    if (vertices.size() < 3)
        return false;

    // Using ray casting method.
    int n = vertices.size();
    bool result = false;
    for (int i = 0; i < n; i++) {
        Point A = vertices[i];
        Point B = vertices[(i+1) % n];

        // Point is right on one of the vertices.
        if ((p.x == A.x && p.y == A.y) || (p.x == B.x && p.y == B.y))
            return false;

        // When a vertex locates on the ray, it is counted as above the ray.
        // If A and B in different side of a ray, we think there is an intersection.
        if ((A.y >= p.y && B.y < p.y) || (A.y < p.y && B.y >= p.y)) {
            double intersection_x = A.x + 1.0 * (p.y-A.y) * (A.x-B.x) / (A.y-B.y);

            // Point is on one of the edges.
            if (qAbs(intersection_x - p.x) < 0.5) {
                return false;
            }

            if (intersection_x > p.x) {
                result = !result;
            }
        }
    }
    return result;
}
