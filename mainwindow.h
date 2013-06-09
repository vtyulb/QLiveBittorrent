#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include <libtorrent/session.hpp>
#include <boost/filesystem/operations.hpp>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    libtorrent::session *session;

    void initSession();
private slots:
    void addTorrent();
    void showInform();
};

#endif // MAINWINDOW_H
