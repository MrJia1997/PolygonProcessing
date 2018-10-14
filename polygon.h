#ifndef POLYGON_H
#define POLYGON_H

#include<QList>
#include<QColor>

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

    double crossProduct(Vector vec);

};

class Point {
public:
    int x;
    int y;

public:
    Point(): x(0), y(0) {}
    Point(int xi, int yi): x(xi), y(yi) {}

    Vector operator-(Point p);
};

class SimplePolygon {
public:
    QList<Point> vertices;
    QColor edgeColor = QColor(0, 0, 0);

public:
    SimplePolygon() {}
    SimplePolygon(QList<Point> vertices) {this->vertices = vertices;}
    ~SimplePolygon() {vertices.clear();}

    int isClockwise();
    void reverseVertices();
};

class Polygon {
public:
    SimplePolygon outerRing;
    QList<SimplePolygon> innerRings;
    QColor innerColor = QColor(255, 255, 0);
    bool isVisible = true;

public:
    Polygon() {}
    Polygon(SimplePolygon o, QList<SimplePolygon> i = QList<SimplePolygon>()): outerRing(o), innerRings(i) {}
    ~Polygon() {innerRings.clear();}

    int isInsidePolygon(Point p);

};
#endif // POLYGON_H
