#include "renderarea.h"
#include <QtMath>
#include <QVector>
#include <QColor>
#include <QPen>
#include <QPainter>
#include <QMessageBox>

RenderArea::RenderArea(QWidget *parent) : QWidget (parent) {

    int areaHeight = this->size().height();
    int areaWidth = this->size().width();
    qDebug() << "Render Area Height:" << areaHeight;
    qDebug() << "Render Area Width:" << areaWidth;

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);

}

int RenderArea::getPolygonNum() {
    return polygons.size();
}

QColor RenderArea::getPolygonFillColor(int id) {
    if (id < 0)
        return QColor();

    return polygons[id].fillColor;
}

QColor RenderArea::getPolygonEdgeColor(int id) {
    if (id < 0)
        return QColor();

    return polygons[id].outerRing.edgeColor;
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

void RenderArea::setGraphLayer(int id) {
    qDebug() << "Current graph layer set to" << id;
    curGraphLayer = id;
}

void RenderArea::setStatus(int status) {
    curStatus = status;
}

void RenderArea::setPolygonFillColor(int id, QColor color) {
    if (id < 0)
        return;
    polygons[id].fillColor = color;
}

void RenderArea::setPolygonEdgeColor(int id, QColor color) {
    if (id < 0)
        return;
    polygons[id].outerRing.edgeColor = color;
    for (int i = 0; i < polygons[id].innerRings.size(); i++) {
        polygons[id].innerRings[i].edgeColor = color;
    }
}

void RenderArea::eraseCurrentPolygon(){
    polygons[curGraphLayer] = Polygon();
    update();
}

void RenderArea::horizontallyFlip(){
    if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
        QMessageBox::warning(this, QString("Warning"), QString("You must have a polygon before flipping."));
        return;
    }

    polygons[curGraphLayer].horizontalFlip();

    update();
}

void RenderArea::verticallyFlip() {
    if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
        QMessageBox::warning(this, QString("Warning"), QString("You must have a polygon before flipping."));
        return;
    }

    polygons[curGraphLayer].verticalFlip();

    update();
}

void RenderArea::clip(int id1, int id2) {
    QList<Polygon> result = Polygon::clip(polygons[id1], polygons[id2]);
    polygons += result;
    for (int i = 0; i < result.size(); i++) {
        emit newPolygonCreated();
    }
}

void RenderArea::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse Pressed @" << event->pos();
    //qDebug() << "Is in the polygon:" << polygons[curGraphLayer].isInsidePolygon(curMousePos);

    if (curStatus == DRAW_OUTER_RING) {
        if (polygons[curGraphLayer].outerRing.vertices.size() > 0) {
            QMessageBox::warning(this, QString("Warning"), QString("You must erase before draw a new one."));
            return;
        }

        curMousePos = Point(event->pos().x(), event->pos().y());

        // Close the polygon path
        if (tempPolygonPath.size() >= 3 && (curMousePos - tempPolygonPath[0]).module() < 10) {
            curStatus = DEFAULT;
            SimplePolygon sp(tempPolygonPath);
            sp.edgeColor = polygons[curGraphLayer].outerRing.edgeColor;
            Polygon p(sp);
            p.fillColor = polygons[curGraphLayer].fillColor;
            polygons[curGraphLayer] = Polygon(sp);

            tempPolygonPath.clear();
            emit polygonPathClosed();
        }
        else {
            tempPolygonPath.append(curMousePos);
        }

    }
    else if (curStatus == DRAW_INNER_RING) {
        if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
            QMessageBox::warning(this, QString("Warning"), QString("You must have outer ring before drawing inner ring."));
            return;
        }

        curMousePos = Point(event->pos().x(), event->pos().y());

        Polygon afterP = polygons[curGraphLayer].afterTransformation();

        if (!afterP.isInsidePolygon(curMousePos)) {
            QMessageBox::warning(this, QString("Warning"), QString("Inner ring must inside the outer ring."));
            tempPolygonPath.clear();
            return;
        }

        // Close the polygon path
        if (tempPolygonPath.size() >= 3 && (curMousePos - tempPolygonPath[0]).module() < 10) {
            curStatus = DEFAULT;
            SimplePolygon sp(tempPolygonPath);
            sp.edgeColor = polygons[curGraphLayer].outerRing.edgeColor;
            afterP.innerRings.append(sp);

            polygons[curGraphLayer] = afterP;

            tempPolygonPath.clear();
            emit polygonPathClosed();
        }
        else {
            tempPolygonPath.append(curMousePos);
        }

    }
    else if (curStatus == MOVE) {
        if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
            QMessageBox::warning(this, QString("Warning"), QString("You must have a polygon before moving."));
            return;
        }

        pressMousePos = Point(event->pos().x(), event->pos().y());
        tempPolygon = polygons[curGraphLayer];
        startMove = true;
    }
    else if (curStatus == ROTATE) {
        if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
            QMessageBox::warning(this, QString("Warning"), QString("You must have a polygon before rotating."));
            return;
        }

        pressMousePos = Point(event->pos().x(), event->pos().y());
        tempPolygon = polygons[curGraphLayer];
        startRotate = true;
    }

    update();
}

void RenderArea::mouseMoveEvent(QMouseEvent *event) {
    curMousePos = Point(event->pos().x(), event->pos().y());

    if (curStatus == MOVE && startMove) {
        int deltaX = curMousePos.x - pressMousePos.x;
        int deltaY = curMousePos.y - pressMousePos.y;

        // Do translation.
        Polygon translateResult = tempPolygon;
        translateResult.translate(deltaX, deltaY);
        polygons[curGraphLayer] = translateResult;
    }
    else if (curStatus == ROTATE && startRotate == true) {
        Point center = tempPolygon.getCenter();

        // Calculate counter-clockwise rotate angle beta.
        Vector vPress = pressMousePos - center;
        Vector vCur = curMousePos - center;

        if (vPress.module() < 1e-5 || vCur.module() < 1e-5)
            return;

        double sinB = 1.0 * (vPress ^ vCur) / (vPress.module() * vCur.module());
        double cosB = 1.0 * (vPress * vCur) / (vPress.module() * vCur.module());

        // Do rotation
        Polygon rotateResult = tempPolygon;
        rotateResult.rotate(sinB, cosB);
        polygons[curGraphLayer] = rotateResult;

    }

    update();
}

void RenderArea::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "Mouse Released @" << event->pos();

    if (curStatus == MOVE && startMove)
        startMove = false;
    else if (curStatus == ROTATE && startRotate)
        startRotate = false;
}

void RenderArea::wheelEvent(QWheelEvent *event) {
    qDebug() << "Mouse Wheel Changed @" << event->delta();
    if (curStatus == ZOOM) {
        if (event->delta() < 0)
            polygons[curGraphLayer].zoom(1.05);
        else
            polygons[curGraphLayer].zoom(0.95);
    }

    update();
}

void RenderArea::paintEvent(QPaintEvent *event) {
    paintFrame();

    for (int i = 0; i < polygons.size(); i++) {
        if (polygons[i].isVisible) {
            paintPolygon(polygons[i]);
        }
    }

    if (curStatus == DRAW_OUTER_RING || curStatus == DRAW_INNER_RING)
        paintTempPolygonPath();
}

void RenderArea::paintFrame() {
    QPainter painter(this);
    QPen pen(QColor(142, 142, 142), 1, Qt::SolidLine);
    painter.setPen(pen);

    painter.drawRect(0, 0, this->size().width() - 1, this->size().height() - 1);

}
void RenderArea::paintTempPolygonPath() {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(Qt::black, 2, Qt::SolidLine);
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
    Polygon afterP = p.afterTransformation();

    int areaWidth = this->size().width();
    int areaHeight = this->size().height();
    QList<Point> frameVertices = {
        Point(0, 0),
        Point(0, areaHeight),
        Point(areaWidth, areaHeight),
        Point(areaWidth, 0)
    };
    SimplePolygon sp(frameVertices);
    Polygon frame(sp);

    QList<Polygon> windowClip = Polygon::clip(afterP, frame);

    // Paint inner area
    for (int i = 0; i < windowClip.size(); i++) {
        windowClip[i].fillColor = afterP.fillColor;
        fillInnerArea(windowClip[i]);
    }
    // Paint edges
    paintEdges(afterP.outerRing);
    for (int i = 0; i < afterP.innerRings.size(); i++)
        paintEdges(afterP.innerRings[i]);

}

void RenderArea::paintEdges(SimplePolygon sp) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(sp.edgeColor, 2, Qt::SolidLine);
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
    QPen pen(p.fillColor, 1, Qt::SolidLine);
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
    SimplePolygon outer = p.outerRing;
    QList<SimplePolygon> inners = p.innerRings;
    QVector<Edge*> NET(areaHeight + 1);
    Edge *AET;

    int ymaxAll = INT_MIN, yminAll = INT_MAX;
    for (int i = 0; i < outer.vertices.size(); i++) {
        if (outer.vertices[i].y > ymaxAll) {
            ymaxAll = outer.vertices[i].y;
        }
        if (outer.vertices[i].y < yminAll) {
            yminAll = outer.vertices[i].y;
        }
    }
    for (int i = 0; i <= ymaxAll; i++)
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
                // First compare x, second compare deltax.
                if ((e->x > insertPos->next->x) ||
                    (qAbs(e->x - insertPos->next->x) < 1e-5 && e->deltax > insertPos->next->deltax)) {
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
            double xA = p->next->x;
            double xB = p->next->next->x;
            painter.drawLine(static_cast<int>(xA + 1), y, static_cast<int>(xB - 1), y);

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
