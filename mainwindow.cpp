#include "mainwindow.h"
#include "clipdialog.h"
#include "ui_mainwindow.h"
#include "ui_clipdialog.h"
#include <QMessageBox>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(960, 640);
    createToolbarActionGroup();
    createRenderArea();
    initGraphLayer();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initGraphLayer() {

    connect(ui->graphLayerList, SIGNAL(currentRowChanged(int)), this, SLOT(onChangeGraphLayer(int)));
    connect(ui->addGraphLayer, SIGNAL(clicked()), this, SLOT(addNewGraphLayer()));
    connect(ui->deleteGraphLayer, SIGNAL(clicked()), this, SLOT(deleteGraphLayer()));
}

void MainWindow::createToolbarActionGroup() {
    toolbarActions = new QActionGroup(this);
    toolbarActions->addAction(ui->actionOuter);
    toolbarActions->addAction(ui->actionInner);
    toolbarActions->addAction(ui->actionPalette);
    toolbarActions->addAction(ui->actionErase);
    toolbarActions->addAction(ui->actionMove);
    toolbarActions->addAction(ui->actionRotate);
    toolbarActions->addAction(ui->actionZoom);
    toolbarActions->addAction(ui->actionHflip);
    toolbarActions->addAction(ui->actionVflip);
    toolbarActions->addAction(ui->actionClip);

    connect(toolbarActions, SIGNAL(triggered(QAction*)), this, SLOT(onClickToolbarActionGroup(QAction*)));
}

void MainWindow::createRenderArea() {
    polygonRender = new RenderArea;
    ui->horizontalLayout->addWidget(polygonRender);
    ui->horizontalLayout->setStretch(0, 1);
    ui->horizontalLayout->setStretch(1, 4);

    connect(polygonRender, &RenderArea::polygonPathClosed, this, &MainWindow::restoreToolbar);
    connect(polygonRender, &RenderArea::newPolygonCreated, this, [&]() {
        QListWidgetItem *newItem = new QListWidgetItem;
        QString layerName = QString("New Layer ") + QString::number(newLayerCounter);
        newItem->setText(layerName);
        ui->graphLayerList->addItem(newItem);
        ui->graphLayerList->setCurrentItem(newItem);
        graphLayerNames.append(layerName);
        newLayerCounter++;
        restoreToolbar();
        polygonRender->clearTempPolygonPath();
        polygonRender->update();
    });
}

void MainWindow::restoreToolbar() {
    if (toolbarActions->checkedAction())
        toolbarActions->checkedAction()->setChecked(false);
}

void MainWindow::onClickToolbarActionGroup(QAction *action) {
    polygonRender->clearTempPolygonPath();
    polygonRender->setStatus(DEFAULT);

    // No valid layer
    if (ui->graphLayerList->currentRow() < 0)
    {
        QMessageBox::warning(this, QString("Warning"), QString("No graph layer selected."));
        restoreToolbar();
        return;
    }


    if (action == ui->actionOuter) {
        polygonRender->setStatus(DRAW_OUTER_RING);
    }
    else if (action == ui->actionInner) {
        polygonRender->setStatus(DRAW_INNER_RING);
    }
    else if (action == ui->actionPalette) {
        int graphLayerId = ui->graphLayerList->currentRow();

        QMessageBox::information(this, QString("Change Color"), QString("Please choose color filled in the polygon."));
        QColor fillColor = QColorDialog::getColor(polygonRender->getPolygonFillColor(graphLayerId),this);
        QMessageBox::information(this, QString("Change Color"), QString("Please choose color of the polygon edges."));
        QColor edgeColor = QColorDialog::getColor(polygonRender->getPolygonEdgeColor(graphLayerId),this);

        polygonRender->setPolygonFillColor(graphLayerId, fillColor);
        polygonRender->setPolygonEdgeColor(graphLayerId, edgeColor);

        restoreToolbar();
    }
    else if (action == ui->actionErase) {
        polygonRender->eraseCurrentPolygon();
        restoreToolbar();
    }
    else if (action == ui->actionMove) {
        polygonRender->setStatus(MOVE);
    }
    else if (action == ui->actionRotate) {
        polygonRender->setStatus(ROTATE);
    }
    else if (action == ui->actionZoom) {
        polygonRender->setStatus(ZOOM);
    }
    else if (action == ui->actionHflip) {
        polygonRender->horizontallyFlip();
        restoreToolbar();
    }
    else if (action == ui->actionVflip) {
        polygonRender->verticallyFlip();
        restoreToolbar();
    }
    else if (action == ui->actionClip) {
        clipDialog = new ClipDialog(this);
        QStringList layers(graphLayerNames);
        clipDialog->addComboItems(layers, SUB_COMBO);
        clipDialog->addComboItems(layers, CLIP_COMBO);
        clipDialog->show();

        connect(clipDialog, &ClipDialog::selectedLayers, this, [&](int idSub, int idClip) {
            polygonRender->clip(idSub, idClip);
        });

        restoreToolbar();
    }
}

void MainWindow::onChangeGraphLayer(int id) {
    qDebug() << "Change to Graph Layer" << id;
    polygonRender->setGraphLayer(id);
    polygonRender->setStatus(DEFAULT);
    restoreToolbar();
}

void MainWindow::addGraphLayer(QString layerName) {
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(layerName);
    ui->graphLayerList->addItem(newItem);
    ui->graphLayerList->setCurrentItem(newItem);
    polygonRender->addPolygon();
    graphLayerNames.append(layerName);

    restoreToolbar();
    polygonRender->clearTempPolygonPath();
    polygonRender->update();
}

void MainWindow::addNewGraphLayer() {
    QString title = QString("New Layer ") + QString::number(newLayerCounter);
    newLayerCounter++;
    addGraphLayer(title);
}

void MainWindow::deleteGraphLayer() {
    int id = ui->graphLayerList->currentRow();
    if (id < 0) {
        QMessageBox::warning(this, QString("Warning"), QString("There are no graph layers."));
        return;
    }
    QListWidgetItem *item = ui->graphLayerList->takeItem(id);
    delete item;
    polygonRender->deletePolygon(id);
    graphLayerNames.takeAt(id);

    // Delete will change graph layer to the next one, which will cause problem.
    polygonRender->setGraphLayer(id);

    restoreToolbar();
    polygonRender->clearTempPolygonPath();
    polygonRender->update();
}
