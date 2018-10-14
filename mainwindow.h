#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderarea.h"
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
    int newLayerCounter = 0;
    RenderArea *polygonRender;
    QActionGroup *toolbarActions;


private:
    void initGraphLayer();
    void createToolbarActionGroup();
    void createRenderArea();

private slots:
    void restoreToolbar();
    void onClickToolbarActionGroup(QAction *action);
    void onChangeGraphLayer(int id);
    void addGraphLayer();
    void deleteGraphLayer();
};

#endif // MAINWINDOW_H
