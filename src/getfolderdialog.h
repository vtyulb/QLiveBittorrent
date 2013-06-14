#ifndef GETFOLDERDIALOG_H
#define GETFOLDERDIALOG_H

#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QHBoxLayout>

class GetFolderDialog : public QGroupBox
{
    Q_OBJECT
public:
    explicit GetFolderDialog(QWidget *parent = 0);
    QPushButton *button;
    QLineEdit *line;
private slots:
    void chooseFolder();
};

#endif // GETFOLDERDIALOG_H
