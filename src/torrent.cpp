#include "torrent.h"

Torrent::Torrent(const QString &path, const QString &mount, torrent_handle handle, QObject *parent) :
    QObject(parent)
{
    qDebug() << mount << path;
    QDir tmp;
    tmp.mkdir(mount);
    torrent = new torrent_handle(handle);
    torrent->set_max_connections(15);

    name = QString::fromStdString(handle.name());
    qDebug() << tmp.mkpath(mount);
    torrent_info inform = handle.get_torrent_info();
    int cnt = 0;
    for (file_iterator i = inform.begin_files(); i != inform.end_files(); i++, cnt++) {
        m["/" + QString::fromStdString(inform.files().at(i).path)] = cnt;
        qDebug() << "added path: " << "/" + QString::fromStdString(inform.files().at(i).path);
    }

    umountList << "-u" << "-o" << "hard_remove" << mount;
    umount();
    mountProcess = new QProcess;
    QObject::connect(mountProcess, SIGNAL(readyRead()), this, SLOT(needPiece()));
    QStringList params;
    params << "-s"; //single-threaded
    params << "-f"; //force don't know what
    params << mount;
    params << path;
    qDebug() << params;
    mountProcess->start(driver, params);
    lastAsk = 0;
    priorities = new bool[torrent->get_torrent_info().num_pieces()];
    for (int i = 0; i < torrent->get_torrent_info().num_pieces(); i++)
        priorities[i] = 0;

    staticReprioritize = new QTimer;
    staticReprioritize->setInterval(500);
    QObject::connect(staticReprioritize, SIGNAL(timeout()), this, SLOT(staticRecall()));
    QTimer::singleShot(120000, this, SLOT(lesserPeers()));
    staticReprioritize->start();
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
    idString = idString.left(idString.length() - 1);
    int id = m[idString];
    long long offset = readInt(mountProcess->readLine());
    long long size = readInt(mountProcess->readLine());

    peer_request req = torrent->get_torrent_info().map_file(id, offset, size);

    int start = req.piece;
    int end = min(start + req.length / torrent->get_torrent_info().piece_length() + 1,
                  torrent->get_torrent_info().num_pieces() - 1);

    lastAsk = start;

    for (int i = 0; i < torrent->get_torrent_info().num_pieces(); i++) {
        torrent->reset_piece_deadline(i);
        priorities[i] = 0;
        if (i == start - 1)
            i = end;
    }

    for (int i = start; i <= end; i++) {
        torrent->set_piece_deadline(i, 100);
        priorities[i] = 1;
    }


    qDebug() << "waiting for piece in range" << torrent->get_torrent_info().num_pieces();
    qDebug() << start << end;
    waitForDownload(start, end);
    qDebug() << "piece gotten";

    mountProcess->write("1\n");
}

long long Torrent::readInt(const QString &s) {
    return s.left(s.length() - 1).toLongLong();
}

void Torrent::waitForDownload(int start, int end) {
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
    for (int i = start; i <= end; i++)
        if (!bit[i])
            return false;

    return true;
}

void Torrent::staticRecall() {
    qDebug() << "recall called";
    int i;
    libtorrent::bitfield bit = torrent->status().pieces;
    int num_pieces = torrent->get_torrent_info().num_pieces();
    for (i = lastAsk; i < num_pieces; i++)
        if (!bit[i])
            break;

    for (int j = i; j < i + 3; j++) {
        torrent->set_piece_deadline(j, 500);
        priorities[j] = 1;
    }
    qDebug() << "recall finished";
}

void Torrent::lesserPeers() {
    qDebug() << "lesser!";
    torrent->set_max_connections(5);
}
