#ifndef SETMOUNTPATHDIALOG_H
#define SETMOUNTPATHDIALOG_H

#include <QDialog>

namespace Ui {
class SetMountPathDialog;
}

class SetMountPathDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SetMountPathDialog(QString defaultMountPath, QWidget *parent = 0);
    ~SetMountPathDialog();
    
private:
    Ui::SetMountPathDialog *ui;
private slots:
    void sendRejected();
    void sendAccepted();
signals:
    void rejected();
    void accepted(QString);
};

#endif // SETMOUNTPATHDIALOG_H
