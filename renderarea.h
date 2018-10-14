#ifndef RENDERAREA_H
#define RENDERAREA_H

#include "polygon.h"
#include <QWidget>


class RenderArea : public QWidget {

    Q_OBJECT

public:
    RenderArea(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;


private:

    QList<Polygon> polygons;

private:
    void paintPolygon(Polygon p);
    void paintEdges(SimplePolygon sp);
    void fillInnerArea(Polygon p);
};

#endif // RENDERAREA_H
