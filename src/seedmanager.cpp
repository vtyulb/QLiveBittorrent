#include "seedmanager.h"

SeedManager::SeedManager(QObject *parent) :
    QObject(parent)
{
    qDebug() << "constuct";
    session = new libtorrent::session;
    findTorrents();

    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();

    initscr();
}

void SeedManager::findTorrents() {
    QDir dir(settingsPath);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++)
        if (list[i].fileName().right(16) == QString(".qlivebittorrent"))
            addTorrent(list[i].fileName());
}

void SeedManager::addTorrent(QString torrent) {
    QFile in(settingsPath + torrent);
    if (!in.open(QIODevice::ReadOnly))
        qDebug() << "Can not read" << torrent;
    else {
        QString torrentName = in.readLine(10000); torrentName = torrentName.left(torrentName.length() - 1);
        QString savePath = in.readLine(10000);    savePath = savePath.left(savePath.length() - 1);
        QString fastResume = in.readLine(10000);  fastResume = fastResume.left(fastResume.length() - 1);

        add_torrent_params p;
        torrent_info *inf = new libtorrent::torrent_info((settingsPath + torrentName).toStdString());
        p.save_path = (savePath + QString::fromStdString(inf->name()) + "/").toStdString();
        p.ti = inf;

        QFile resumeData(settingsPath + fastResume);
        resumeData.open(QIODevice::ReadOnly);
        std::vector<char> *v = new std::vector<char>;
        QByteArray data = resumeData.readAll();
        v->resize(data.size());
        for (int i = 0; i < data.size(); i++)
            (*v)[i] = data[i];
        p.resume_data = v;

        session->add_torrent(p);
    }
}

void SeedManager::updateInform() {
    erase();
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();
    printw("Seeding on %d torrents:\n", v.size());
    for (int i = 0; i < v.size(); i++) {
        printw("%d: %s; u - %d; uploaded - %dM; state - ", i, v[i].name().c_str(), v[i].status().upload_payload_rate, v[i].status().total_payload_upload / 1000000);
        printw("%s\n", getNormalStatus(v[i].status().state).toLocal8Bit().constData());
    }

    refresh();
}
