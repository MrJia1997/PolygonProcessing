#include "renderarea.h"
#include <QtMath>
#include <QVector>
#include <QColor>
#include <QPen>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>

RenderArea::RenderArea(QWidget *parent) : QWidget (parent) {

    int areaHeight = this->size().height();
    int areaWidth = this->size().width();
    qDebug() << "Render Area Height:" << areaHeight;
    qDebug() << "Render Area Width:" << areaWidth;

//    // Example vertices;
//    QList<Point> vertices;
//    vertices.append(Point(100, 100));
//    vertices.append(Point(50, 150));
//    vertices.append(Point(100, 200));
//    vertices.append(Point(150, 150));
//    SimplePolygon outer(vertices);
//    QList<Point> vertices2;
//    vertices2.append(Point(100, 125));
//    vertices2.append(Point(75, 150));
//    vertices2.append(Point(100, 175));
//    vertices2.append(Point(125, 150));
//    SimplePolygon inner1(vertices2);
//    QList<SimplePolygon> inner;
//    inner.append(inner1);

//    Polygon p(outer, inner);
//    p.innerColor = QColor(255, 0, 0);
//    polygons.append(p);


    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);

}

int RenderArea::getPolygonNum() {
    return polygons.size();
}

void RenderArea::addPolygon() {
    polygons.append(Polygon());
}

void RenderArea::deletePolygon(int id) {
    polygons.removeAt(id);
}

void RenderArea::clearTempPolygonPath() {
    tempPolygonPath.clear();
}

void RenderArea::changeGraphLayer(int id) {
    curGraphLayer = id;
}

void RenderArea::changeStatus(int status) {
    curStatus = status;
}

void RenderArea::eraseCurrentPolygon(){
    polygons[curGraphLayer] = Polygon();
}

void RenderArea::horizontallyFlip(){
    // TODO: Add horizontally flip.
}

void RenderArea::verticallyFlip() {
    // TODO: Add vertically flip.
}

void RenderArea::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse Pressed @" << event->pos();
    if (curStatus == DRAW_OUTER_RING) {
        if (polygons[curGraphLayer].outerRing.vertices.size() > 0) {
            QMessageBox::warning(this, QString("Warning"), QString("You must erase before draw a new one."));
            return;
        }

        curMousePos = Point(event->pos().x(), event->pos().y());
        auto distance = [](Point A, Point B) {
            return qSqrt((A.x-B.x) * (A.x-B.x) + (A.y-B.y) * (A.y-B.y));
        };

        // Close the polygon path
        if (tempPolygonPath.size() >= 3 && distance(curMousePos, tempPolygonPath[0]) < 3) {
            curStatus = DEFAULT;
            SimplePolygon sp(tempPolygonPath);
            polygons[curGraphLayer] = Polygon(sp);

            tempPolygonPath.clear();
            emit polygonPathClosed();
        }
        else {
            tempPolygonPath.append(curMousePos);
        }

    }

    update();
}

void RenderArea::mouseMoveEvent(QMouseEvent *event) {
    curMousePos = Point(event->pos().x(), event->pos().y());
    update();
}

void RenderArea::mouseReleaseEvent(QMouseEvent *event) {

}

void RenderArea::paintEvent(QPaintEvent *event) {
    qDebug() << "Paint Event Triggered.";

    paintFrame();
    for (int i = 0; i < polygons.size(); i++) {
        if (polygons[i].isVisible) {
            qDebug() << "Draw Polygon " << i;
            paintPolygon(polygons[i]);
        }
    }

    if (curStatus == DRAW_OUTER_RING || curStatus == DRAW_INNER_RING)
        paintTempPolygonPath();
}

void RenderArea::paintFrame() {
    QPainter painter(this);
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(pen);

    painter.drawRect(0, 0, this->size().width() - 1, this->size().height() - 1);

}
void RenderArea::paintTempPolygonPath() {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(pen);

    int n = tempPolygonPath.size();
    for (int i = 1; i < n; i++) {
        Point p1 = tempPolygonPath[i - 1];
        Point p2 = tempPolygonPath[i];
        painter.drawLine(p1.x, p1.y, p2.x, p2.y);
    }

    if (n > 0) {
        Point end = tempPolygonPath[n-1];
        painter.drawLine(end.x, end.y, curMousePos.x, curMousePos.y);
    }

}


void RenderArea::paintPolygon(Polygon p) {
    // Paint edges
    paintEdges(p.outerRing);
    for (int i = 0; i<p.innerRings.size(); i++)
        paintEdges(p.innerRings[i]);

    // Paint inner area
    fillInnerArea(p);

}

void RenderArea::paintEdges(SimplePolygon sp) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(sp.edgeColor, 1, Qt::SolidLine);
    painter.setPen(pen);
    int v = sp.vertices.size();
    for (int i = 0; i < v; i++) {
        Point v1 = sp.vertices[i];
        Point v2 = sp.vertices[(i + 1) % v];
        painter.drawLine(v1.x, v1.y, v2.x, v2.y);
    }

}

void RenderArea::fillInnerArea(Polygon p) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(p.innerColor, 1, Qt::SolidLine);
    painter.setPen(pen);

    // Using Line Sweep Algorithm
    struct Edge {
        double x;
        double deltax;
        int ymax;
        Edge *next;

        Edge(): next(nullptr) {}
    };

    int areaHeight = this->size().height();

    QVector<Edge*> NET(areaHeight);
    Edge *AET;
    SimplePolygon outer = p.outerRing;
    QList<SimplePolygon> inners = p.innerRings;

    int ymaxAll = INT_MIN, yminAll = INT_MAX;
    for (int i = 0; i < outer.vertices.size(); i++) {
        if (outer.vertices[i].y > ymaxAll) {
            ymaxAll = outer.vertices[i].y;
        }
        if (outer.vertices[i].y < yminAll) {
            yminAll = outer.vertices[i].y;
        }
    }
    for (int i = 0; i < ymaxAll; i++)
        NET[i] = new Edge;
    AET = new Edge;

    // Add edges to NET
    auto addNET = [&](SimplePolygon sp) {
        int n = sp.vertices.size();
        for (int i = 0; i < n; i++) {
            Point p_prev_A = sp.vertices[(i-1+n)%n];
            Point p_A = sp.vertices[i];
            Point p_B = sp.vertices[(i+1)%n];
            Point p_next_B = sp.vertices[(i+2)%n];

            // Ignore horizontal lines
            if (p_A.y == p_B.y)
                continue;

            int ymin = qMin(p_A.y, p_B.y);
            int ymax = qMax(p_A.y, p_B.y);
            double x = p_A.y < p_B.y ? p_A.x : p_B.x;
            double deltax = 1.0 * (p_A.x - p_B.x) / (p_A.y - p_B.y);

            // Deal with vertices like this:
            //     ..      ..
            //   ..    or    ..
            //     ..      ..
            if ((p_A.y == ymin && p_prev_A.y < ymin) ||
                (p_B.y == ymin && p_next_B.y < ymin)) {
                ymin++;
                x += deltax;
            }

            // Insert into NET
            Edge *p = new Edge;
            p->x = x;
            p->deltax = deltax;
            p->ymax = ymax;
            p->next = NET[ymin]->next;
            NET[ymin]->next = p;
        }
    };

    addNET(outer);
    for (int i = 0; i < inners.size(); i++)
        addNET(inners[i]);

    // Start sweeping using AET
    for (int y = yminAll; y < ymaxAll; y++) {

        // Insert NET into AET
        while(NET[y]->next) {
            Edge *e = NET[y]->next;
            Edge *insertPos = AET;

            while (insertPos->next) {
                if (e->x > insertPos->next->x) {
                    insertPos = insertPos->next;
                    continue;
                }
                break;
            }

            NET[y]->next = e->next;
            e->next = insertPos->next;
            insertPos->next = e;
        }

        // Fill color
        Edge *p = AET;
        while (p->next && p->next) {
            double x_A = p->next->x;
            double x_B = p->next->next->x;
            for (int x = static_cast<int>(x_A + 1); x < x_B; x++)
                painter.drawPoint(x, y);
            p = p->next->next;
        }

        // Delete invalid edges
        p = AET;
        while (p->next) {
            if (p->next->ymax <= y) {
                Edge *q = p->next;
                p->next = q->next;
                q->next = nullptr;
                delete q;
            }
            else {
                p = p->next;
            }
        }

        // Add up x
        p = AET;
        while (p->next) {
            p->next->x += p->next->deltax;
            p = p->next;
        }
    }
}
