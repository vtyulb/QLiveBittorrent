#include "seedmanager.h"

SeedManager::SeedManager(QObject *parent) :
    QObject(parent)
{
    session = new libtorrent::session;
    findTorrents();

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();

    QTimer *checkForErrorsTimer = new QTimer(this);
    checkForErrorsTimer->setInterval(120000);
    QObject::connect(checkForErrorsTimer, SIGNAL(timeout()), this, SLOT(checkForErrors()));
    checkForErrorsTimer->start();

    initscr();
}

SeedManager::~SeedManager() {
    endwin();
    session->pause();
    std::vector<torrent_handle> v = session->get_torrents();
    for (int i = 0; i < v.size(); i++) {
        std::deque<alert *> trash;
        session->pop_alerts(&trash);
        v[i].save_resume_data(torrent_handle::save_info_dict);
        const alert *a = session->wait_for_alert(libtorrent::seconds(3));
        if (a == NULL)
            qDebug() << "Can not save resume data";

        std::auto_ptr<alert> holder = session->pop_alert();
        if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a))
            qDebug() << "Failed alert";

        const libtorrent::save_resume_data_alert *rd = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a);
        if (rd == 0)
            qDebug() << "Very big fail";

        QSettings s(settingsPath + QString::fromStdString(v[i].name()) + ".qlivebittorrent", QSettings::IniFormat);
        s.setValue("data", QVariant(saveResumeData(rd)));
        s.sync();
    }

    delete session;
}

void SeedManager::findTorrents() {
    QDir dir(settingsPath);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++)
        if (list[i].fileName().right(16) == QString(".qlivebittorrent"))
            addTorrent(list[i].fileName());
}

void SeedManager::addTorrent(QString torrent) {
    QSettings s(settingsPath + torrent, QSettings::IniFormat);
    QByteArray data = s.value("data").toByteArray();
    libtorrent::entry e = libtorrent::bdecode(data.begin(), data.end());
    libtorrent::torrent_info *inf = new libtorrent::torrent_info(e);
    const libtorrent::torrent_handle h =
           session->add_torrent(inf, (s.value("path").toString() + QString::fromStdString(inf->name()) + "/").toStdString(), e);

    torrentNames[h.name()] = torrent;
}

void SeedManager::updateInform() {
    erase();
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();

    printw("%s\n", ("/ №|" + setStringSize("Name", 20) + "|Upload speed(KB/s)|Uploaded(MB)|Size(MB)|" +
                   setStringSize("state", 11) + "\\").toLocal8Bit().constData());
    for (int i = 0; i < v.size(); i++) {
        printw("|%s|%s|%s|%s|%s|",
               setStringSize(QString::number(i), 2).toLocal8Bit().constData(),
               setStringSize(QString::fromStdString(v[i].name()), 20).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().upload_payload_rate / 1000), 18).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().total_payload_upload / 1000000), 12).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].get_torrent_info().total_size() / 1000000), 8).toLocal8Bit().constData());
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
