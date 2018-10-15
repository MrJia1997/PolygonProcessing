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

    double meanY = 0.0;
    for (int i = 0; i < polygons[curGraphLayer].outerRing.vertices.size(); i++) {
        meanY += polygons[curGraphLayer].outerRing.vertices[i].y;
    }
    meanY /= polygons[curGraphLayer].outerRing.vertices.size();


    int intMeanY = static_cast<int>(meanY);
    Polygon translateResult = polygons[curGraphLayer];
    for (int i = 0; i < translateResult.outerRing.vertices.size(); i++) {
        translateResult.outerRing.vertices[i].y = 2 * intMeanY - translateResult.outerRing.vertices[i].y;
    }
    for (int i = 0; i < translateResult.innerRings.size(); i++) {
        for (int j = 0; j < translateResult.innerRings[i].vertices.size(); j++) {
            translateResult.innerRings[i].vertices[j].y = 2 * intMeanY - translateResult.innerRings[i].vertices[j].y;
        }
    }
    polygons[curGraphLayer] = translateResult;

    update();
}

void RenderArea::verticallyFlip() {
    if (polygons[curGraphLayer].outerRing.vertices.size() == 0) {
        QMessageBox::warning(this, QString("Warning"), QString("You must have a polygon before flipping."));
        return;
    }

    double meanX = 0.0;
    for (int i = 0; i < polygons[curGraphLayer].outerRing.vertices.size(); i++) {
        meanX += polygons[curGraphLayer].outerRing.vertices[i].x;
    }
    meanX /= polygons[curGraphLayer].outerRing.vertices.size();


    int intMeanX = static_cast<int>(meanX);
    Polygon translateResult = polygons[curGraphLayer];
    for (int i = 0; i < translateResult.outerRing.vertices.size(); i++) {
        translateResult.outerRing.vertices[i].x = 2 * intMeanX - translateResult.outerRing.vertices[i].x;
    }
    for (int i = 0; i < translateResult.innerRings.size(); i++) {
        for (int j = 0; j < translateResult.innerRings[i].vertices.size(); j++) {
            translateResult.innerRings[i].vertices[j].x = 2 * intMeanX - translateResult.innerRings[i].vertices[j].x;
        }
    }
    polygons[curGraphLayer] = translateResult;

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

        if (!polygons[curGraphLayer].outerRing.isInsideSimplePolygon(curMousePos)) {
            QMessageBox::warning(this, QString("Warning"), QString("Inner ring must inside the outer ring."));
            tempPolygonPath.clear();
            return;
        }

        // Close the polygon path
        if (tempPolygonPath.size() >= 3 && (curMousePos - tempPolygonPath[0]).module() < 10) {
            curStatus = DEFAULT;
            SimplePolygon sp(tempPolygonPath);
            sp.edgeColor = polygons[curGraphLayer].outerRing.edgeColor;
            polygons[curGraphLayer].innerRings.append(sp);

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
            if (p.x < minX)
                minX = p.x;
            if (p.x > maxX)
                maxX = p.x;
            if (p.y < minY)
                minY = p.y;
            if (p.y > maxY)
                maxY = p.y;
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
        for (int i = 0; i < translateResult.outerRing.vertices.size(); i++) {
            translateResult.outerRing.vertices[i].translate(deltaX, deltaY);
        }
        for (int i = 0; i < translateResult.innerRings.size(); i++) {
            for (int j = 0; j < translateResult.innerRings[i].vertices.size(); j++) {
                translateResult.innerRings[i].vertices[j].translate(deltaX, deltaY);
            }
        }

        polygons[curGraphLayer] = translateResult;
    }
    else if (curStatus == ROTATE && startRotate == true) {
        // Calculate rotate center.
        double meanX = 0.0, meanY = 0.0;
        for (int i = 0; i < tempPolygon.outerRing.vertices.size(); i++) {
            meanX += tempPolygon.outerRing.vertices[i].x;
            meanY += tempPolygon.outerRing.vertices[i].y;
        }
        meanX /= tempPolygon.outerRing.vertices.size();
        meanY /= tempPolygon.outerRing.vertices.size();
        Point center(static_cast<int>(meanX), static_cast<int>(meanY));

        // Calculate counter-clockwise rotate angle beta.
        Vector vPress = pressMousePos - center;
        Vector vCur = curMousePos - center;
        double sinB = 1.0 * (vPress ^ vCur) / (vPress.module() * vCur.module());
        double cosB = 1.0 * (vPress * vCur) / (vPress.module() * vCur.module());

        // Do rotation
        Polygon rotateResult = tempPolygon;
        for (int i = 0; i < rotateResult.outerRing.vertices.size(); i++) {
            Vector vPoly = rotateResult.outerRing.vertices[i] - center;
            vPoly.rotate(sinB, cosB);
            rotateResult.outerRing.vertices[i] = center+ vPoly;

        }
        for (int i = 0; i < rotateResult.innerRings.size(); i++) {
            for (int j = 0; j < rotateResult.innerRings[i].vertices.size(); j++) {
                Vector vPoly = rotateResult.innerRings[i].vertices[j] - center;
                vPoly.rotate(sinB, cosB);
                rotateResult.innerRings[i].vertices[j] = center + vPoly;
            }
        }

        polygons[curGraphLayer] = rotateResult;
    }

    update();
}

void RenderArea::mouseReleaseEvent(QMouseEvent *event) {
    if (curStatus == MOVE && startMove)
        startMove = false;
    else if (curStatus == ROTATE && startRotate)
        startRotate = false;
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
