#ifndef RENDERAREA_H
#define RENDERAREA_H

#include "polygon.h"
#include <QWidget>

enum {
    DEFAULT,
    DRAW_OUTER_RING,
    DRAW_INNER_RING,
    MOVE,
    ROTATE,
    ZOOM
};

class RenderArea : public QWidget {

    Q_OBJECT

public:
    RenderArea(QWidget *parent = nullptr);
    int getPolygonNum();
    QColor getPolygonFillColor(int id);
    QColor getPolygonEdgeColor(int id);
    void addPolygon();
    void deletePolygon(int id);
    void clearTempPolygonPath();

signals:
    void polygonPathClosed();

public slots:
    void setGraphLayer(int id);
    void setStatus(int status);
    void setPolygonFillColor(int id, QColor color);
    void setPolygonEdgeColor(int id, QColor color);
    void eraseCurrentPolygon();
    void horizontallyFlip();
    void verticallyFlip();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QList<Polygon> polygons;
    QList<Point> tempPolygonPath;
    Polygon tempPolygon;

    int curGraphLayer = -1;
    int curStatus = DEFAULT;

    Point pressMousePos;
    Point curMousePos;

    bool startMove = false;
    bool startRotate = false;
private:
    void paintFrame();
    void paintPolygon(Polygon p);
    void paintTempPolygonPath();
    void paintEdges(SimplePolygon sp);
    void fillInnerArea(Polygon p);
};

#endif // RENDERAREA_H
