#include "torrent.h"

Torrent::Torrent(const QString &path, const QString &mount, const libtorrent::torrent_handle &handle, QObject *parent) :
    QObject(parent)
{
    qDebug() << mount;
    libtorrent::torrent_info inform = handle.get_torrent_info();
    int cnt = 0;
    for (file_iterator i = inform.begin_files(); i != inform.end_files(); i++, cnt++) {
        m[mount + QString::fromStdString(inform.files().at(i).path)] = cnt;
        qDebug() << "added path: " << QString::fromStdString(inform.files().at(i).path);
    }

    mount = new QProcess;
    QStringList params;
    params << "-s"; //single-threaded
    params << "-f"; //force don't know what
    params << mount; //where mounting
    params << path; //what mounting
    mount->start("driver", params);
}
