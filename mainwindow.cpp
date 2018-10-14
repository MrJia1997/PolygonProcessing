#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(960, 640);
    createToolbarActionGroup();
    createRenderArea();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createToolbarActionGroup() {
    toolbarActions = new QActionGroup(this);
    toolbarActions->addAction(ui->action_polygon);
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

}

void MainWindow::onClickToolbarActionGroup(QAction *action) {

}
