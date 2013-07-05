#include "torrentdialog.h"
#include "ui_torrentdialog.h"

TorrentDialog::TorrentDialog(QString name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TorrentDialog)
{
    ui->setupUi(this);
    ui->groupBox->setTitle("Download path");
    ui->groupBox_2->setTitle("Mount path");

    QSettings s(settingsPath + "qlivebittorrent.ini");
    ui->groupBox->line->setText(s.value("download", QVariant(QDir::homePath())).toString());
    ui->groupBox_2->line->setText(s.value("mount", QVariant(QDir::homePath() + "mount")). toString());

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(finish()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    torrentName = name;
}

TorrentDialog::~TorrentDialog() {
    QSettings s(settingsPath + "qlivebittorrent.ini");
    s.setValue("download", QVariant(ui->groupBox->line->text()));
    s.setValue("mount", QVariant(ui->groupBox_2->line->text()));
    delete ui;
}

void TorrentDialog::finish() {
    emit success(torrentName, ui->groupBox->line->text(), ui->groupBox_2->line->text());
    deleteLater();
}

void TorrentDialog::reject() {
    emit rejected();
    deleteLater();
}
