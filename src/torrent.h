#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <libtorrent/torrent_handle.hpp>

using libtorrent::file_storage;
typedef file_storage::iterator file_iterator;

class Torrent : public QObject
{
    Q_OBJECT
public:
    explicit Torrent(const QString &path, const libtorrent::torrent_handle &handle, QObject *parent = 0);
    QMap<QString, int> m;
signals:
    
public slots:
    
};

#endif // TORRENT_H
