#include "torrent.h"

#include <boost/asio/error.hpp>

Torrent::Torrent(const QString &path, const QString &mount, torrent_handle handle, QObject *parent) :
    QObject(parent)
{
    mountProcess = new QProcess;
    downloadPath = path;
    lastAskTime = NULL;
    QDir tmp;
    tmp.mkdir(mount);
    torrent = new torrent_handle(handle);
    torrent->set_max_connections(15);
    waitForMetadata(torrent);

    name = QString::fromStdString(handle.name());
    torrent_info inform = handle.get_torrent_info();
    int cnt = 0;
    for (auto i = inform.begin_files(); i != inform.end_files(); i++, cnt++)
        m["/" + QString::fromStdString(inform.files().at(i).path)] = cnt;

    umountList << "-u" << "-q" << "-o" << "hard_remove" << mount;
    QObject::connect(mountProcess, SIGNAL(readyRead()), this, SLOT(needPiece()));
    mountPath = mount;
    remount();
    lastAsk = 0;
    priorities = new bool[torrent->get_torrent_info().num_pieces()];
    for (int i = 0; i < torrent->get_torrent_info().num_pieces(); i++)
        priorities[i] = 0;

    staticReprioritize = new QTimer;
    staticReprioritize->setInterval(500);
    QObject::connect(staticReprioritize, SIGNAL(timeout()), this, SLOT(staticRecall()));
    QTimer::singleShot(120000, this, SLOT(lesserPeers()));
    staticReprioritize->start();
    num_pieces = inform.num_pieces();
    agressive = true;
}

void Torrent::umount() {
    QProcess umountProc;
    umountProc.execute("fusermount", umountList);
    umountProc.waitForFinished(1000);
}

Torrent::~Torrent() {
    umount();
}

void Torrent::needPiece() {
    if (lastAskTime == NULL)
        lastAskTime = new QTime;
    *lastAskTime = QTime::currentTime();
    char string[1000];
    mountProcess->readLine(string, 999);
    QString idString = QString(string);
    idString = idString.left(idString.length() - 1);
    int id = m[idString];
    long long offset = readInt(mountProcess->readLine());
    long long size = readInt(mountProcess->readLine());

    peer_request req = torrent->get_torrent_info().map_file(id, offset, size);

    int start = req.piece;
    int end = min(start + req.length / torrent->get_torrent_info().piece_length() + 1,
                  num_pieces - 1);

    lastAsk = start;

    for (int i = 0; i < num_pieces; i++) {
        torrent->reset_piece_deadline(i);
        priorities[i] = 0;
        if (i == start - 1)
            i = end;
    }

    for (int i = start; i <= end; i++) {
        torrent->set_piece_deadline(i, 100);
        priorities[i] = 1;
    }

    waitForDownload(start, end);
    assert(mountProcess->isWritable());
    mountProcess->write("1\n");
}

long long Torrent::readInt(const QString &s) {
    return s.left(s.length() - 1).toLongLong();
}

void Torrent::waitForDownload(int start, int end) {
    assert(start >= 0);
    assert(end <= torrent->get_torrent_info().num_pieces());
    while (!checkForDownload(start, end))
        sleep(100);
}

void Torrent::sleep(int ms) {
    QEventLoop *loop = new QEventLoop;
    QTimer::singleShot(ms, loop, SLOT(quit()));
    loop->exec();
}

bool Torrent::checkForDownload(int start, int end) {
    libtorrent::bitfield bit = torrent->status().pieces;
    if (bit.size() < end) {
        printw("very hard error");
        return false;
    }

    for (int i = start; i <= end; i++)
        if (!bit[i])
            return false;

    return true;
}

void Torrent::staticRecall() {
    if (!agressive)
        return;

    int i;
    libtorrent::bitfield bit = torrent->status().pieces;
    if (bit.size() < num_pieces)
        return;

    for (i = lastAsk; i < num_pieces; i++)
        if (!bit[i])
            break;

    for (int j = i; j < num_pieces; j++) {
        torrent->set_piece_deadline(j, 200 + j * 5);
        priorities[j] = 1;
    }
}

void Torrent::lesserPeers() {
    torrent->set_max_connections(5);
}

void Torrent::invertAgressive() {
    if (agressive) {
        for (int i = 0; i < num_pieces; i++)
            torrent->reset_piece_deadline(i);

        torrent->set_max_connections(30);
    } else
        torrent->set_max_connections(6);

    agressive = !agressive;

}

void Torrent::waitForMetadata(const torrent_handle *handle) {
    qDebug() << "Waiting for metadata";
    while (!handle->has_metadata())
        sleep(100);
}

void Torrent::remount() {
    mountProcess->close();
    umount();
    QStringList params;
    params << "-s"; //single-threaded
    params << "-f"; //force don't know what
    params << mountPath;
    params << downloadPath;
    mountProcess->start(driver, params);
}

void Torrent::setMountPath(QString mount) {
    mountPath = mount;
}

bool Torrent::mountStatus() {
    if (this == NULL)
        return false;
    else
        return mountProcess->state() == QProcess::Running;
}
