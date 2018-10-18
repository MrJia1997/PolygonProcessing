#include "clipdialog.h"
#include "ui_clipdialog.h"
#include <QMessageBox>

ClipDialog::ClipDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClipDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickOKButton()));
}

ClipDialog::~ClipDialog()
{
    delete ui;
}

void ClipDialog::addComboItems(QStringList items, int type) {
    if (type == SUB_COMBO)
        ui->subCombo->addItems(items);
    else if (type == CLIP_COMBO)
        ui->clipCombo->addItems(items);
}

void ClipDialog::onClickOKButton() {
    int idSub = ui->subCombo->currentIndex();
    int idClip = ui->clipCombo->currentIndex();
    if (idSub == idClip) {
        QMessageBox::warning(this, QString("Warning"), QString("You must choose different polygons."));
        return;
    }
    emit selectedLayers(idSub, idClip);
}
