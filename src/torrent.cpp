#include "torrent.h"

Torrent::Torrent(const QString &path, const QString &mount, const libtorrent::torrent_handle &handle, QObject *parent) :
    QObject(parent)
{
    qDebug() << mount << path;
    QDir tmp;
    name = QString::fromStdString(handle.name());
    qDebug() << tmp.mkpath(mount);
    libtorrent::torrent_info inform = handle.get_torrent_info();
    int cnt = 0;
    for (file_iterator i = inform.begin_files(); i != inform.end_files(); i++, cnt++) {
        m["/" + QString::fromStdString(inform.files().at(i).path)] = cnt;
        qDebug() << "added path: " << "/" + QString::fromStdString(inform.files().at(i).path);
    }

    QProcess umount;
    umount.execute("fusermount", QStringList("-u") + QStringList(mount));
    if (!umount.waitForFinished(1000))
        qDebug() << "You have a big problem";

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

void Torrent::needPiece() {
    char string[1000];
    mountProcess->readLine(string, 999);
    QString idString = QString(string);
    idString = "/" + name + idString.left(idString.length() - 1);
    int id = m[idString];
    int offset = readInt(mountProcess->readLine());
    int size = readInt(mountProcess->readLine());
    mountProcess->write("1\n");
    qDebug() << id << offset << size;
}

int Torrent::readInt(const QString &s) {
    return s.left(s.length() - 1).toInt();
}
