#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

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
//    int n = polygonRender->getPolygonNum();
//    for (int i = 0; i < 10; i++) {
//        QListWidgetItem *newItem = new QListWidgetItem;
//        newItem->setText(QString("New Layer ") + QString::number(newLayerCounter));
//        ui->graphLayerList->addItem(newItem);
//        newLayerCounter++;
//    }

    connect(ui->graphLayerList, SIGNAL(currentRowChanged(int)), this, SLOT(onChangeGraphLayer(int)));
    connect(ui->addGraphLayer, SIGNAL(clicked()), this, SLOT(addGraphLayer()));
    connect(ui->deleteGraphLayer, SIGNAL(clicked()), this, SLOT(deleteGraphLayer()));
}

void MainWindow::createToolbarActionGroup() {
    toolbarActions = new QActionGroup(this);
    toolbarActions->addAction(ui->action_outer);
    toolbarActions->addAction(ui->action_inner);
    toolbarActions->addAction(ui->action_palette);
    toolbarActions->addAction(ui->action_erase);
    toolbarActions->addAction(ui->action_move);
    toolbarActions->addAction(ui->action_rotate);
    toolbarActions->addAction(ui->action_zoom);
    toolbarActions->addAction(ui->action_hflip);
    toolbarActions->addAction(ui->action_vflip);
    toolbarActions->addAction(ui->action_clip);

    connect(toolbarActions, SIGNAL(triggered(QAction*)), this, SLOT(onClickToolbarActionGroup(QAction*)));
}

void MainWindow::createRenderArea() {
    polygonRender = new RenderArea;
    ui->horizontalLayout->addWidget(polygonRender);
    ui->horizontalLayout->setStretch(0, 1);
    ui->horizontalLayout->setStretch(1, 4);

    connect(polygonRender, &RenderArea::polygonPathClosed, this, &MainWindow::restoreToolbar);
}

void MainWindow::restoreToolbar() {
    if (toolbarActions->checkedAction())
        toolbarActions->checkedAction()->setChecked(false);
}

void MainWindow::onClickToolbarActionGroup(QAction *action) {
    // TODO: Add tool bar actions.
    polygonRender->clearTempPolygonPath();

    if (ui->graphLayerList->currentRow() < 0) // No valid layer
    {
        QMessageBox::warning(this, QString("Warning"), QString("No graph layer selected."));
        restoreToolbar();
        return;
    }
    if (action == ui->action_outer) {
        polygonRender->changeStatus(DRAW_OUTER_RING);
        return;
    }

}

void MainWindow::onChangeGraphLayer(int id) {
    qDebug() << "Change to Graph Layer" << id;
    polygonRender->changeGraphLayer(id);
    polygonRender->changeStatus(DEFAULT);
}

void MainWindow::addGraphLayer() {
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(QString("New Layer ") + QString::number(newLayerCounter));
    ui->graphLayerList->addItem(newItem);
    ui->graphLayerList->setCurrentItem(newItem);
    polygonRender->addPolygon();
    newLayerCounter++;

    restoreToolbar();
    polygonRender->clearTempPolygonPath();
    polygonRender->update();
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

    restoreToolbar();
    polygonRender->clearTempPolygonPath();
    polygonRender->update();
}
