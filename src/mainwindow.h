#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QStandardItemModel>
#include <QSettings>
#include <QPainter>
#include <QMessageBox>
#include <QByteArray>
#include <QMainWindow>
#include <QApplication>
#include <QTime>
#include <QFileInfo>

#include <torrent.h>
#include <generateimage.h>
#include <torrentdialog.h>
#include <constants.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/alert_types.hpp"
#include "boost/filesystem/operations.hpp"
#include "curses.h"

using libtorrent::torrent_info;
using libtorrent::torrent_handle;
using libtorrent::torrent_status;
using libtorrent::add_torrent_params;\
using libtorrent::alert;
using libtorrent::bencode;

Q_DECLARE_METATYPE(libtorrent::entry)

class MainWindow: public QObject
{
    Q_OBJECT
    
public:
    explicit MainWindow(QString torrent, QString downloadPath, QString mountPath, QString rate, bool gui, QObject *parent = 0);
    ~MainWindow();
    
private:
    libtorrent::session *session;
    QMainWindow *fake;
    QByteArray standartText;
    unsigned int standartTextLen;
    Torrent *main;

    QString resumeTorrentName;
    QString resumeSavePath;
    QString resumeName;

    void initSession(QString rate);
    void findPaths(QString torrent);
    void die(QString error);
    void setupTimers();

    bool midnight();
private slots:
    void addTorrent();
    void realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath);
    void updateInform();
    void checkKeys();
    void updateStandartText();
};

#endif // MAINWINDOW_H
