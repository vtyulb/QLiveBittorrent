#include "torrent.h"

Torrent::Torrent(const QString &path, const QString &mount, torrent_handle handle, QObject *parent) :
    QObject(parent)
{
    qDebug() << mount << path;
    QDir tmp;
    torrent = new torrent_handle(handle);

    name = QString::fromStdString(handle.name());
    qDebug() << tmp.mkpath(mount);
    torrent_info inform = handle.get_torrent_info();
    int cnt = 0;
    for (file_iterator i = inform.begin_files(); i != inform.end_files(); i++, cnt++) {
        m["/" + QString::fromStdString(inform.files().at(i).path)] = cnt;
        qDebug() << "added path: " << "/" + QString::fromStdString(inform.files().at(i).path);
    }

    umountList << "-u" << mount;
    umount();
    mountProcess = new QProcess;
    QObject::connect(mountProcess, SIGNAL(readyRead()), this, SLOT(needPiece()));
    QStringList params;
    params << "-s"; //single-threaded
    params << "-f"; //force don't know what
    params << mount;
    params << path;
    qDebug() << params;
    mountProcess->start("./../driver", params);
}

void Torrent::umount() {
    QProcess umountProc;
    umountProc.execute("fusermount", umountList);
    if (!umountProc.waitForFinished(10))
        qDebug() << "You have a big problem";
}

Torrent::~Torrent() {
    umount();
}

void Torrent::needPiece() {
    char string[1000];
    mountProcess->readLine(string, 999);
    QString idString = QString(string);
    idString = "/" + name + idString.left(idString.length() - 1);
    int id = m[idString];
    long long offset = readInt(mountProcess->readLine());
    long long size = readInt(mountProcess->readLine());

    peer_request req = torrent->get_torrent_info().map_file(id, offset, size);


    mountProcess->write("1\n");
    qDebug() << id << offset << size;
}

long long Torrent::readInt(const QString &s) {
    return s.left(s.length() - 1).toLongLong();
}
