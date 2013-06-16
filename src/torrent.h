#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/peer_info.hpp>

using libtorrent::file_storage;
using libtorrent::torrent_info;
using libtorrent::torrent_handle;
using libtorrent::peer_request;
using libtorrent::partial_piece_info;
using std::min;
typedef file_storage::iterator file_iterator;

#ifdef PORTABLE
    const QString driver="./qlivebittorrent-driver";
#else
    const QString driver="qlivebittorrent-driver";
#endif

class Torrent : public QObject
{
    Q_OBJECT
public:
    explicit Torrent(const QString &path, const QString &mount, torrent_handle handle, QObject *parent = 0);
    QMap<QString, int> m;
    ~Torrent();
    static void sleep(int ms);

private:
    QProcess *mountProcess;
    long long readInt(const QString &s);
    void waitForDownload(int start, int end);
    bool checkForDownload(int start, int end);
    void umount();
    torrent_handle *torrent;
    int lastAsk;
    int num_pieces;
    bool *priorities;
    QString name;
    QStringList umountList;
    QTimer *staticReprioritize;
signals:
    
private slots:
    void needPiece();
    void staticRecall();
    void lesserPeers();
};

#endif // TORRENT_H
