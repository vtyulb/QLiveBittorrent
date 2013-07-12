#include "setmountpathdialog.h"
#include "ui_setmountpathdialog.h"

SetMountPathDialog::SetMountPathDialog(QString defaultMountPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetMountPathDialog)
{
    ui->setupUi(this);
    ui->groupBox->line->setText(defaultMountPath);
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(sendRejected()));
    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(sendAccepted()));
}

SetMountPathDialog::~SetMountPathDialog()
{
    delete ui;
}

void SetMountPathDialog::sendAccepted() {
    emit accepted(ui->groupBox->line->text());
}

void SetMountPathDialog::sendRejected() {
    emit rejected();
}
