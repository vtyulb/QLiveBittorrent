#include "getfolderdialog.h"

GetFolderDialog::GetFolderDialog(QWidget *parent) :
    QGroupBox(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    button = new QPushButton("Choose", this);
    line = new QLineEdit(this);
    layout->addWidget(line);
    layout->addWidget(button);
    this->setLayout(layout);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(chooseFolder()));
}

void GetFolderDialog::chooseFolder() {
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    dialog->show();
    QObject::connect(dialog, SIGNAL(fileSelected(QString)), line, SLOT(setText(QString)));
}
