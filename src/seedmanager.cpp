#include "seedmanager.h"

SeedManager::SeedManager(QObject *parent) :
    QObject(parent)
{
    session = new libtorrent::session;
    findTorrents();

    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();

    QTimer *checkForErrorsTimer = new QTimer;
    checkForErrorsTimer->setInterval(120000);
    QObject::connect(checkForErrorsTimer, SIGNAL(timeout()), this, SLOT(checkForErrors()));
    checkForErrorsTimer->start();

    QTimer::singleShot(3000, this, SLOT(deleteLater()));
    initscr();
}

SeedManager::~SeedManager() {
    endwin();
    std::vector<torrent_handle> v = session->get_torrents();
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

        QByteArray data;
        data.resize(1000000);
        bencode(data.begin(), *rd->resume_data);

        QFile file(settingsPath + v[i].get_torrent_info().name().c_str() + ".fastresume");
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }
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
        p.upload_mode = true;

        const libtorrent::torrent_handle h = session->add_torrent(p);
        torrentNames[h.name()] = torrent;
        fastResumeNames[h.name()] = torrent;
    }
}

void SeedManager::updateInform() {
    erase();
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();

    printw("%s\n", ("/ №|" + setStringSize("Name", 20) + "|Upload speed(KB/s)|Uploaded(MB)|Size(MB)|" + setStringSize("state", 11) + "\\").toLocal8Bit().constData());
    for (int i = 0; i < v.size(); i++) {
        printw("%s%s%s%s%s",
               setStringSize(QString::number(i), 2, true).toLocal8Bit().constData(),
               setStringSize(QString::fromStdString(v[i].name()), 20).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().upload_payload_rate / 1000), 18, true).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().total_payload_upload / 1000000), 12).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].get_torrent_info().total_size() / 1000000), 8, true).toLocal8Bit().constData());
        printw("%s|\n", setStringSize(getNormalStatus(v[i].status().state), 11).toLocal8Bit().constData());
    }

    refresh();
}

void SeedManager::checkForErrors() {
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();
    for (int i = 0; i < v.size(); i++)
        if (v[i].status().paused) {
            QFile file(settingsPath + "qlivebittorrent.log");
            file.open(QIODevice::Append);
            QTextStream cout(&file);
            cout << "Torrent " << QString::fromStdString(v[i].name()) << "deleted because of "
                 << QString::fromStdString(v[i].status().error);
            cout.flush();
            file.close();

            QFile::rename(torrentNames[v[i].name()], torrentNames[v[i].name()] + ".deleted");
        }
}
