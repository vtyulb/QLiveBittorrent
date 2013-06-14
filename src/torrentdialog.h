#ifndef TORRENTDIALOG_H
#define TORRENTDIALOG_H

#include <QDialog>
#include <QString>

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
signals:
    void success(QString torrentFile, QString torrentPath, QString mountPath);
    void rejected();
};

#endif // TORRENTDIALOG_H
