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
#include <QUrl>
#include <QMap>
#include <QFont>

#include <torrent.h>
#include <generateimage.h>
#include <torrentdialog.h>
#include <constants.h>
#include <abstractseedingclass.h>
#include <ui_mainwindow.h>

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
#include "libtorrent/magnet_uri.hpp"
#include "boost/filesystem/operations.hpp"
#include "curses.h"

using libtorrent::torrent_info;
using libtorrent::torrent_handle;
using libtorrent::torrent_status;
using libtorrent::add_torrent_params;
using libtorrent::alert;
using libtorrent::bencode;

Q_DECLARE_METATYPE(libtorrent::entry)

class MainWindow: public QObject, public AbstractSeedingClass
{
    Q_OBJECT
    
public:
    explicit MainWindow(QString torrent, QString downloadPath, QString mountPath, QString rate, bool gui, QObject *parent = 0);
    ~MainWindow();
    bool informationSaved();

private:
    libtorrent::session *session;
    QMainWindow *fake;
    QMap<std::string, Torrent *> mapTorrent;
    QSet<QString> s;
    Ui_MainWindow *ui;
    QByteArray standartText;
    unsigned int standartTextLen;
    Torrent *main;
    bool informationFlushed;

    QString resumeSavePath;

    void initSession(QString rate);
    void findPaths(QString torrent);
    void die(QString error);
    void setupTimers();
    void findTorrents();
    void addTorrentByName(QString torrent);

    bool midnight();
    bool hasGUI();
private slots:
    void addTorrent();
    void realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath);
    void updateInform();
    void checkKeys();
    void updateStandartText();
    void updateTable();
    void remountRequest();
};

#endif // MAINWINDOW_H
