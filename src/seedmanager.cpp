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

SeedManager::~SeedManager() {
    /*std::vector<torrent_handle> v = session->get_torrents();
    for (int i = 0; i < v.size(); i++) {
        std::deque<alert *> trash;
        session->pop_alerts(&trash);
        v[i].save_resume_data();
        const alert *a = session->wait_for_alert(libtorrent::seconds(3));
        if (a == NULL)
            qDebug() << "Can not save resume data";

        std::auto_ptr<alert> holder = session->pop_alert();
        if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a))
            qDebug() << "Failed alert";

        const libtorrent::save_resume_data_alert *rd = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a);
        if (rd == 0)
            qDebug() << "Very big fail";


        std::ofstream out((settingsPath + v[i].get_torrent_info().name().c_str() + ".fastresume").toLocal8Bit(), std::ios_base::binary);
        bencode(std::ostream_iterator<char>(out), *rd->resume_data);
        out.flush();

        qDebug() << resumeName;
        QFile fout(settingsPath + resumeName + ".qlivebittorrent");
        fout.open(QIODevice::WriteOnly);
        QTextStream cout(&fout);
        cout << resumeTorrentName << "\n" << resumeSavePath << "\n" << resumeName + ".fastresume" << "\n";
        cout.flush();
    }*/
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
        p.flags = add_torrent_params::flag_seed_mode;
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
