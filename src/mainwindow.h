#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QStandardItemModel>
#include <torrent.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/session.hpp"
#include "boost/filesystem/operations.hpp"

namespace Ui {
    class MainWindow;
}

using libtorrent::torrent_info;
using libtorrent::torrent_handle;
using libtorrent::torrent_status;
using libtorrent::add_torrent_params;

const QString mountPath = "/home/vlad/mount/";
const QString savePath = "/home/vlad/test/";

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
    void updateInform();
};

#endif // MAINWINDOW_H
