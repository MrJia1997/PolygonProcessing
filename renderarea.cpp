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

void RenderArea::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse Pressed @" << event->pos();

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
        //qDebug() << "Is in the outer ring:" << polygons[curGraphLayer].outerRing.isInsideSimplePolygon(curMousePos);

        Polygon afterP = polygons[curGraphLayer].afterTransformation();

        if (!afterP.outerRing.isInsideSimplePolygon(curMousePos)) {
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

        // Prevent from moving out of the frame.
        int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
        for (int i = 0; i < tempPolygon.outerRing.vertices.size(); i++) {
            Point p = tempPolygon.outerRing.vertices[i];
            if (p.x < minX) minX = p.x;
            if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
        }

        int areaHeight = this->size().height();
        int areaWidth = this->size().width();

        if (deltaX > 0)
            deltaX = qMin(deltaX, areaWidth - maxX - 2);
        else
            deltaX = qMax(deltaX, -minX + 2);
        if (deltaY > 0)
            deltaY = qMin(deltaY, areaHeight - maxY - 2);
        else
            deltaY = qMax(deltaY, -minY + 2);

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
    // qDebug() << "Paint Event Triggered.";

    paintFrame();

    for (int i = 0; i < polygons.size(); i++) {
        if (polygons[i].isVisible) {
            // qDebug() << "Draw Polygon " << i;
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

    // Paint edges
    paintEdges(afterP.outerRing);
    for (int i = 0; i < afterP.innerRings.size(); i++)
        paintEdges(afterP.innerRings[i]);

    // Paint inner area
    fillInnerArea(afterP);

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
    QVector<Edge*> NET(areaHeight);
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
            double x_A = p->next->x;
            double x_B = p->next->next->x;
            for (int x = static_cast<int>(x_A + 2); x < (x_B - 2); x++)
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
