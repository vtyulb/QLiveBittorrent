#ifndef TORRENTDIALOG_H
#define TORRENTDIALOG_H

#include <QDialog>
#include <QString>
#include <QSettings>
#include <QDir>
#include <constants.h>

namespace Ui {
class TorrentDialog;
}

class TorrentDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TorrentDialog(QString name, QWidget *parent = 0);
    ~TorrentDialog();
private:
    Ui::TorrentDialog *ui;
    QString torrentName;
private slots:
    void finish();
    void reject();
signals:
    void success(QString torrentFile, QString torrentPath, QString mountPath);
    void rejected();
};

#endif // TORRENTDIALOG_H
