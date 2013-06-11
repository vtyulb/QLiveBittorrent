#include "torrent.h"

Torrent::Torrent(const QString &path, const QString &mount, torrent_handle handle, QObject *parent) :
    QObject(parent)
{
    qDebug() << mount << path;
    QDir tmp;
    tmp.mkdir(mount);
    torrent = new torrent_handle(handle);
//    torrent->set_sequential_download(true);
    torrent->set_download_limit(200000);
    torrent->set_max_connections(10);

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
    idString = idString.left(idString.length() - 1);
    int id = m[idString];
    long long offset = readInt(mountProcess->readLine());
    long long size = readInt(mountProcess->readLine());

    peer_request req = torrent->get_torrent_info().map_file(id, offset, size);
    std::vector<int> priorities = torrent->piece_priorities();
    for (int i = 0; i < priorities.size(); i++)
        if (priorities[i] != 7)
            if ((i >= req.piece) && (i <= req.piece + 5))
                priorities[i] = 7;
            else
                priorities[i] = 1;

    torrent->prioritize_pieces(priorities);

    int start = req.piece;
    int end = min(start + req.length / torrent->get_torrent_info().piece_length() + 1,
                  torrent->get_torrent_info().num_pieces() - 1);

    qDebug() << "waiting for piece";
    qDebug() << start << end;
    waitForDownload(start, end);
    qDebug() << "piece gotten";

    mountProcess->write("1\n");
}

long long Torrent::readInt(const QString &s) {
    return s.left(s.length() - 1).toLongLong();
}

void Torrent::waitForDownload(int start, int end) {
    while (!checkForDownload(start, end)) {
        std::vector<partial_piece_info> queue;
        torrent->get_download_queue(queue);
        for (int i = 0; i < queue.size(); i++) {
            printf("%d %d\n", queue[i].piece_index, queue[i].piece_state);
            fflush(stdout);
        }
        sleep(100);
    }
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
