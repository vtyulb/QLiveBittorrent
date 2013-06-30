#include "torrentdialog.h"
#include "ui_torrentdialog.h"

TorrentDialog::TorrentDialog(QString name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TorrentDialog)
{
    ui->setupUi(this);
    ui->groupBox->setTitle("Download path");
    ui->groupBox_2->setTitle("Mount path");


    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(finish()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(deleteLater()));
    torrentName = name;
}

TorrentDialog::~TorrentDialog()
{
    emit rejected();
    delete ui;
}

void TorrentDialog::finish() {
    emit success(torrentName, ui->groupBox->line->text(), ui->groupBox_2->line->text());
}
