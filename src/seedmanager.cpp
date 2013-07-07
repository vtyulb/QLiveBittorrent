#include "seedmanager.h"

SeedManager::SeedManager(QString rate, QObject *parent) :
    QObject(parent)
{
    session = new libtorrent::session;
    session->listen_on(std::make_pair(6881, 6889));
    session->set_alert_mask(0);
    session->set_upload_rate_limit(rate.toInt() * 1000);
    findTorrents();

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();

    QTimer *checkForErrorsTimer = new QTimer(this);
    checkForErrorsTimer->setInterval(120000);
    QObject::connect(checkForErrorsTimer, SIGNAL(timeout()), this, SLOT(checkForErrors()));
    checkForErrorsTimer->start();

    QTimer *findNewTorrents = new QTimer(this);
    findNewTorrents->setInterval(600 * 1000);
    QObject::connect(findNewTorrents, SIGNAL(timeout()), this, SLOT(findTorrents()));
    findNewTorrents->start();

    QTimer *checkKeys = new QTimer(this);
    checkKeys->setInterval(10);
    QObject::connect(checkKeys, SIGNAL(timeout()), this, SLOT(checkKeys()));
    checkKeys->start();

    initscr();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    noecho();
}

SeedManager::~SeedManager() {
    endwin();
    qDebug() << "saving information about torrents";
    std::vector<torrent_handle> v = session->get_torrents();
    for (int i = 0; i < v.size(); i++) {
        std::deque<alert *> trash;
        session->pop_alerts(&trash);
        v[i].save_resume_data(torrent_handle::save_info_dict);
        const alert *a = session->wait_for_alert(libtorrent::seconds(3));
        if (a == NULL) {
            qDebug() << "Can not save resume data";
            break;
        }

        std::auto_ptr<alert> holder = session->pop_alert();
        if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a)) {
            qDebug() << "Failed alert";
            break;
        }

        const libtorrent::save_resume_data_alert *rd = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a);
        if (rd == 0) {
            qDebug() << "Very big fail";
            break;
        }

        QSettings s(settingsPath + QString::fromStdString(v[i].name()) + ".qlivebittorrent", QSettings::IniFormat);
        s.setValue("data", QVariant(saveResumeData(rd)));
        s.sync();
    }

    informationFlushed = true;
    qDebug() << "sending information about (upload/download) sizes to trackers";
    session->pause();
    delete session;
}

void SeedManager::findTorrents() {
    QDir dir(settingsPath);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++)
        if (!s.contains(list[i].fileName()))
            if (list[i].fileName().right(16) == QString(".qlivebittorrent")) {
                addTorrent(list[i].fileName());
                s.insert(list[i].fileName());
            }
}

void SeedManager::addTorrent(QString torrent) {
    QSettings s(settingsPath + torrent, QSettings::IniFormat);
    QByteArray data = s.value("data").toByteArray();
    libtorrent::entry e = libtorrent::bdecode(data.begin(), data.end());
    libtorrent::torrent_info *inf = new libtorrent::torrent_info(e);
    const libtorrent::torrent_handle h =
            session->add_torrent(inf, (s.value("path").toString() + QString::fromStdString(inf->name()) + "/").toStdString(), e);

    h.set_upload_mode(true);
    h.auto_managed(false);
    if (h.is_paused())
        h.resume();

    torrentNames[h.name()] = torrent;
}

void SeedManager::updateInform() {
    erase();
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();

    printw("%s\n", ("/ №|" + setStringSize("Name", 20) + "|Upload speed(KB/s)|Uploaded(MB)|Size(MB)|" +
                   setStringSize("state", 11) + "\\").toLocal8Bit().constData());
    for (int i = firstDisplayingTorrent; i < firstDisplayingTorrent + numberDisplayingTorrents(); i++) {
        printw("|%s|%s|%s|%s|%s|",
               setStringSize(QString::number(i + 1), 2).toLocal8Bit().constData(),
               setStringSize(QString::fromStdString(v[i].name()), 20).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().upload_payload_rate / 1000), 18).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].status().all_time_upload / 1000000), 12).toLocal8Bit().constData(),
               setStringSize(QString::number(v[i].get_torrent_info().total_size() / 1000000), 8).toLocal8Bit().constData());
        printw("%s|\n", setStringSize(getNormalStatus(v[i].status().state), 11).toLocal8Bit().constData());
    }

    if ((firstDisplayingTorrent + numberDisplayingTorrents() < v.size()))
        if (firstDisplayingTorrent)
            printw("Arrows to navigate (up and down)\n");
        else
            printw("Arrows to navigate (only down)\n");
    else if (firstDisplayingTorrent)
        printw("Arrow to navigate (only up)\n");

    if (session->upload_rate_limit() != 0)
        printw("Upload rate limit: %d(KB/s)", session->upload_rate_limit() / 1000);

    refresh();
}

void SeedManager::checkForErrors() {
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();
    for (int i = 0; i < v.size(); i++)
        if (v[i].status().paused) {
            QFile file(settingsPath + "qlivebittorrent.log");
            file.open(QIODevice::Append);
            QTextStream cout(&file);
            cout << "Torrent " << QString::fromStdString(v[i].name()) << " deleted because of "
                 << QString::fromStdString(v[i].status().error) << "\n----------------";
            cout.flush();
            file.close();

//            session->remove_torrent(v[i]);
            v[i].resume();
            QFile::rename(settingsPath + torrentNames[v[i].name()], settingsPath + torrentNames[v[i].name()] + ".deleted");
        }
}

bool SeedManager::informationSaved() {
    return informationFlushed;
}

int SeedManager::numberDisplayingTorrents() {
    return min(stdscr->_maxy - 4, int(session->get_torrents().size()));
}

void SeedManager::checkKeys() {
    int key = wgetch(stdscr);
    if (key == ERR)
        return;

    if (key == KEY_DOWN)
        firstDisplayingTorrent = min(int(session->get_torrents().size() -
                                numberDisplayingTorrents()), firstDisplayingTorrent + 1);
    else if (key == KEY_UP)
        firstDisplayingTorrent = max(0, firstDisplayingTorrent - 1);
    else if (key == '+')
        session->set_upload_rate_limit(session->upload_rate_limit() + 10000);
    else if (key == '-')
        session->set_upload_rate_limit(session->upload_rate_limit() - 10000);

    updateInform();
}
