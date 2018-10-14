#include "polygon.h"

double Vector::crossProduct(Vector vec) {
    return x * vec.y - y * vec.x;
}

Vector Point::operator-(Point p) {
    return Vector(x - p.x, y - p.y);
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
    double xp = v1.crossProduct(v2);

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

int Polygon::isInsidePolygon(Point p) {

    return 0;
}
