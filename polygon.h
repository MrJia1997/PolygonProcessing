#ifndef POLYGON_H
#define POLYGON_H

#include <QList>
#include <QColor>
#include <QGenericMatrix>

enum {
    CLOCKWISE,
    COUNTERCLOCKWISE
};

enum {
    OUTSIDE = -1,
    ON_BOUNDARY,
    INSIDE
};

class Vector {
public:
    int x;
    int y;

public:
    Vector(): x(0), y(0) {}
    Vector(int xi, int yi): x(xi), y(yi) {}

    friend Vector operator*(double a, Vector v);
    int operator*(Vector v);
    int operator^(Vector v);

    double module();
    void rotate(double sinB, double cosB);
};

class Point {
public:
    int x;
    int y;

public:
    Point(): x(0), y(0) {}
    Point(int xi, int yi): x(xi), y(yi) {}

    Point operator+(Vector v);
    Vector operator-(Point p);
    void translate(Vector delta);
    void translate(int deltaX, int deltaY);
};

class SimplePolygon {
public:
    QList<Point> vertices;
    QColor edgeColor = QColor(0, 0, 0);

public:
    SimplePolygon() {}
    SimplePolygon(QList<Point> vertices) {this->vertices = vertices;}

    int isClockwise();
    void reverseVertices();
    bool isInsideSimplePolygon(Point p);

    static SimplePolygon afterTransformation(SimplePolygon sp, QGenericMatrix<3, 3, double> transformation);
};

class Polygon {
public:
    SimplePolygon outerRing;
    QList<SimplePolygon> innerRings;
    QGenericMatrix<3, 3, double> transformation;
    QColor fillColor = QColor(255, 255, 255, 0);
    bool isVisible = true;

public:
    Polygon() {}
    Polygon(SimplePolygon o, QList<SimplePolygon> i = QList<SimplePolygon>()): outerRing(o), innerRings(i) {}
    ~Polygon() {innerRings.clear();}

    Point getCenter();
    void translate(int deltaX, int deltaY);
    void rotate(double sinB, double cosB);
    void zoom(double scale);
    void horizontalFlip();
    void verticalFlip();

    Polygon afterTransformation();

};
#endif // POLYGON_H
