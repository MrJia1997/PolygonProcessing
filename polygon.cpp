#define ZOOM_MAX 5.0
#define ZOOM_MIN 0.2

#include "polygon.h"
#include <QtMath>


// Multiplication by scalar
Vector operator*(double a, Vector v) {
    Vector res;
    res.x = static_cast<int>(a * v.x);
    res.y = static_cast<int>(a * v.y);
    return res;
}

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

bool Polygon::isInsidePolygon(Point p) {
    if (outerRing.vertices.size() < 3)
        return false;

    // Using ray casting method.

    bool end = false;
    bool result = false;

    auto calcIntersections = [&](QList<Point> vertices, Point p) {
        int n = vertices.size();
        for (int i = 0; i < n; i++) {
            Point A = vertices[i];
            Point B = vertices[(i+1) % n];

            // Point is right on one of the vertices.
            if ((p.x == A.x && p.y == A.y) || (p.x == B.x && p.y == B.y)) {
                result = false;
                end = true;
                return;
            }
            // When a vertex locates on the ray, it is counted as above the ray.
            // If A and B in different side of a ray, we think there is an intersection.
            if ((A.y >= p.y && B.y < p.y) || (A.y < p.y && B.y >= p.y)) {
                double intersection_x = A.x + 1.0 * (p.y-A.y) * (A.x-B.x) / (A.y-B.y);

                // Point is on one of the edges.
                if (qAbs(intersection_x - p.x) < 0.5) {
                    result = false;
                    end = true;
                    return;
                }

                if (intersection_x > p.x) {
                    result = !result;
                }
            }
        }
        return;
    };

    calcIntersections(outerRing.vertices, p);
    for (int i = 0; i < innerRings.size(); i++) {
        if (end == true)
            return result;
        calcIntersections(innerRings[i].vertices, p);
    }
    return result;
}

Point Polygon::getCenter() {
    Polygon afterP = this->afterTransformation();

    double meanX = 0.0, meanY = 0.0;
    int verticesNum = afterP.outerRing.vertices.size();
    for (int i = 0; i < verticesNum; i++) {
        meanX += afterP.outerRing.vertices[i].x;
        meanY += afterP.outerRing.vertices[i].y;
    }
    meanX /= verticesNum;
    meanY /= verticesNum;
    return Point(static_cast<int>(meanX), static_cast<int>(meanY));
}

void Polygon::translate(int deltaX, int deltaY) {
    double transValue[] = {
        1, 0, static_cast<double>(deltaX),
        0, 1, static_cast<double>(deltaY),
        0, 0, 1
    };
    transformation =  QGenericMatrix<3, 3, double>(transValue) * transformation;
    return;
}

void Polygon::rotate(double sinB, double cosB) {
    Point center = getCenter();
    double transValue1[] = {
        1, 0, static_cast<double>(-center.x),
        0, 1, static_cast<double>(-center.y),
        0, 0, 1
    };
    double rotValue[] = {
        cosB, -sinB, 0,
        sinB,  cosB, 0,
           0,     0, 1
    };
    double transValue2[] = {
        1, 0, static_cast<double>(center.x),
        0, 1, static_cast<double>(center.y),
        0, 0, 1
    };
    QGenericMatrix<3, 3, double> trans1(transValue1), trans2(transValue2), rot(rotValue);
    transformation = trans2 * rot * trans1 * transformation;

    return;
}

void Polygon::zoom(double scale) {
    auto detTrans = [](QGenericMatrix<3, 3, double> mat){
        double *data = mat.data();
        return data[0] * data[4] - data[1] * data[3];
    };

    double scaleNow = detTrans(transformation);
    double scaleAdjusted = 1.0;

    if (scaleNow * scale > ZOOM_MAX)
        scaleAdjusted = ZOOM_MAX / scaleNow;
    else if (scaleNow * scale < ZOOM_MIN)
        scaleAdjusted = ZOOM_MIN / scaleNow;
    else
        scaleAdjusted = scale;


    Point center = getCenter();

    double transValue1[] = {
        1, 0, static_cast<double>(-center.x),
        0, 1, static_cast<double>(-center.y),
        0, 0, 1
    };
    double zoomValue[] = {
        scaleAdjusted,             0, 0,
                    0, scaleAdjusted, 0,
                    0,             0, 1
    };
    double transValue2[] = {
        1, 0, static_cast<double>(center.x),
        0, 1, static_cast<double>(center.y),
        0, 0, 1
    };

    QGenericMatrix<3, 3, double> trans1(transValue1), trans2(transValue2), zoom(zoomValue);
    transformation = trans2 * zoom * trans1 * transformation;
    qDebug() << "Scale:" << detTrans(transformation);

    return;
}

void Polygon::horizontalFlip() {
    Point center = getCenter();

    double transValue1[] = {
        1, 0, 0,
        0, 1, static_cast<double>(-center.y),
        0, 0, 1
    };
    double flipValue[] = {
        1,  0, 0,
        0, -1, 0,
        0,  0, 1
    };
    double transValue2[] = {
        1, 0, 0,
        0, 1, static_cast<double>(center.y),
        0, 0, 1
    };
    QGenericMatrix<3, 3, double> trans1(transValue1), trans2(transValue2), flip(flipValue);
    transformation = trans2 * flip * trans1 * transformation;

    return;
}

void Polygon::verticalFlip() {
    Point center = getCenter();

    double transValue1[] = {
        1, 0, static_cast<double>(-center.x),
        0, 1, 0,
        0, 0, 1
    };
    double flipValue[] = {
       -1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };
    double transValue2[] = {
        1, 0, static_cast<double>(center.x),
        0, 1, 0,
        0, 0, 1
    };
    QGenericMatrix<3, 3, double> trans1(transValue1), trans2(transValue2), flip(flipValue);
    transformation = trans2 * flip * trans1 * transformation;

    return;
}

SimplePolygon SimplePolygon::afterTransformation(SimplePolygon sp, QGenericMatrix<3, 3, double> transformation) {
    SimplePolygon result = sp;
    for (int i = 0; i < result.vertices.size(); i++) {
        double values[] = {
            static_cast<double>(result.vertices[i].x),
            static_cast<double>(result.vertices[i].y),
            1
        };
        QGenericMatrix<1, 3, double> point(values);
        QGenericMatrix<1, 3, double> after = transformation * point;
        result.vertices[i].x = static_cast<int>(after.data()[0]);
        result.vertices[i].y = static_cast<int>(after.data()[1]);
    }
    return result;
}

Polygon Polygon::afterTransformation() {
    Polygon result = *this;
    result.outerRing = SimplePolygon::afterTransformation(outerRing, transformation);
    for (int i = 0; i < result.innerRings.size(); i++)
        result.innerRings[i] = SimplePolygon::afterTransformation(innerRings[i], transformation);
    result.transformation.setToIdentity();

    return result;
}
