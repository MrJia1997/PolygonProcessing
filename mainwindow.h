#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderarea.h"
#include <QMainWindow>
#include <QActionGroup>


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
    RenderArea *polygonRender;
    QActionGroup *toolbarActions;


private:
    void createToolbarActionGroup();
    void createRenderArea();

private slots:
    void onClickToolbarActionGroup(QAction *action);
};

#endif // MAINWINDOW_H
