#ifndef SEEDMANAGER_H
#define SEEDMANAGER_H

#include <QObject>
#include <QString>
#include <QFileInfoList>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QSettings>
#include <QSet>

#include <curses.h>
#include <map>
#include <iostream>

#include <constants.h>
#include <abstractseedingclass.h>

#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/torrent_info.hpp>

//#include <boost/asio/impl/src.hpp>

using libtorrent::add_torrent_params;
using libtorrent::torrent_info;
using libtorrent::torrent_status;
using libtorrent::torrent_handle;
using libtorrent::entry;
using libtorrent::alert;

using std::min;
using std::max;

class SeedManager : public QObject, public AbstractSeedingClass
{
    Q_OBJECT
public:
    explicit SeedManager(QString rate, QObject *parent = 0);
    ~SeedManager();
    bool informationSaved();

private:
    bool informationFlushed;
    int firstDisplayingTorrent;
    libtorrent::session *session;
    std::map<std::string, QString> torrentNames;
    QSet<QString> s;

    int numberDisplayingTorrents();
    void addTorrent(QString);
private slots:
    void updateInform();
    void checkForErrors();
    void findTorrents();
    void checkKeys();
};

#endif // SEEDMANAGER_H
