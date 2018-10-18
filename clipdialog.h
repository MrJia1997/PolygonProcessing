#ifndef CLIPDIALOG_H
#define CLIPDIALOG_H

#include <QDialog>

enum {
    SUB_COMBO,
    CLIP_COMBO
};

namespace Ui {
class ClipDialog;
}

class ClipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClipDialog(QWidget *parent = nullptr);
    ~ClipDialog();

    void addComboItems(QStringList items, int type);

signals:
    void selectedLayers(int, int);

private:
    Ui::ClipDialog *ui;

private slots:
    void onClickOKButton();

};

#endif // CLIPDIALOG_H
