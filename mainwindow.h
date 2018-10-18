#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderarea.h"
#include "clipdialog.h"
#include <QMainWindow>
#include <QActionGroup>
#include <QListWidget>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ClipDialog *clipDialog;
    int newLayerCounter = 0;
    int clipLayerCounter = 0;
    RenderArea *polygonRender;
    QList<QString> graphLayerNames;
    QActionGroup *toolbarActions;


private:
    void initGraphLayer();
    void createToolbarActionGroup();
    void createRenderArea();

private slots:
    void restoreToolbar();
    void onClickToolbarActionGroup(QAction *action);
    void onChangeGraphLayer(int id);
    void addGraphLayer(QString layerName);
    void addNewGraphLayer();
    void deleteGraphLayer();
};

#endif // MAINWINDOW_H
