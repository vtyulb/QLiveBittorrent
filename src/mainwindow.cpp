#include "mainwindow.h"

MainWindow::MainWindow(QString torrent, QString downloadPath, QString mountPath, QString rate, bool gui, QObject *parent): QObject(parent) {
    initSession(rate);
    initscr();
    nodelay(stdscr, true);
    noecho();
    main = NULL;
    if (!gui)
        realAddTorrent(torrent, downloadPath, mountPath);
    else {
        fake = new QMainWindow;
        if (QFile(torrent).exists())
            findPaths(torrent);
        else
            addTorrent();
    }
}

MainWindow::~MainWindow() {
    endwin();
}

void MainWindow::initSession(QString rate) {
    session = new libtorrent::session;
    libtorrent::session_settings settings = session->settings();
    settings.max_allowed_in_request_queue = 4;
    settings.seed_choking_algorithm = settings.fastest_upload;
    settings.choking_algorithm = settings.bittyrant_choker;
    session->set_settings(settings);
    session->set_download_rate_limit(rate.toInt() * 1000);
}

void MainWindow::addTorrent() {
    QString torrent = QFileDialog::getOpenFileName(fake, QString(), QString(),
                                                   QString("*.torrent"));
    if (QFile(torrent).exists())
        findPaths(torrent);
    else
        die("User don't choose torrent file");
}

void MainWindow::findPaths(QString torrent) {
    TorrentDialog *dialog = new TorrentDialog(torrent, fake);
    dialog->show();
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), this, SLOT(realAddTorrent(QString, QString, QString)));
    QObject::connect(dialog, SIGNAL(rejected()), qApp, SLOT(quit()));
}

void MainWindow::updateStandartText() {
    QString text;
    if (session->download_rate_limit() != 0)
        text += QString("Download rate limit: %1KB/s\n").arg(session->download_rate_limit() / 1000);

    if (main != NULL) {
        text += "Status: ";
        if (main->torrent->status().state == torrent_status::downloading)
            text += "downloading\n";
        else if (main->torrent->status().state == torrent_status::finished)
            text += "finished\n";
        else if (main->torrent->status().state == torrent_status::seeding)
            text += "seeding\n";
        else if (main->torrent->status().state == torrent_status::checking_files)
            text += "checking files\n";
        else
            text += "Calculating first 1000000000! digits of PI\n";
    }

    for (int i = 1; i < stdscr->_maxx; i++)
        text += '=';
    text += '\n';

    standartText = standartText.left(standartTextLen) + text.toLocal8Bit();
}

void MainWindow::realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath) {
    if (!QFile::exists(torrentFile))
        die("torrent file not found");

    standartText = ("Torrent file: " + torrentFile + "\nDownload path: " + torrentPath + "\nMount path: " + mountPath + "\n").toLocal8Bit();
    standartTextLen = standartText.size();
    updateStandartText();

    if (torrentPath[torrentPath.length() - 1] != QChar('/'))
        torrentPath += "/";
    if (mountPath[mountPath.length() - 1] != QChar('/'))
        mountPath += "/";
    add_torrent_params p;
    torrent_info *inf = new libtorrent::torrent_info(torrentFile.toStdString());
    p.save_path = (torrentPath + QString::fromStdString(inf->name()) + "/").toStdString();
    p.ti = inf;
    p.storage_mode = libtorrent::storage_mode_allocate;

    main = new Torrent(torrentPath + QString::fromStdString(inf->name()), mountPath + QString::fromStdString(inf->name()), session->add_torrent(p), this);
    setupTimers();
}

void MainWindow::updateInform() {
    /*std::vector<torrent_handle> v = session->get_torrents();

    for (unsigned int i = 0; i < v.size(); i++) {
        torrent_status s = v[i].status();
        torrent_info inf = v[i].get_torrent_info();
        std::vector<partial_piece_info> tmp;
        v[i].get_download_queue(tmp);
        std::vector<libtorrent::partial_piece_info> inform;
        v[i].get_download_queue(inform);
        if (inform.size())
            for (unsigned int i = 0; i < inform.size(); i++)
                qDebug() << inform[i].piece_index << inform[i].piece_state;
    }*/


    clear();
    libtorrent::torrent_status status = main->torrent->status();
    libtorrent::torrent_info info = main->torrent->get_torrent_info();
    printw("%s", standartText.constData());
    printw("%d of %d peers connected; %d of %d MB downloaded; progress - %d\%; dspeed - %dKB/s; uspeed - %dKB/s\n",
           status.num_connections, status.list_seeds + status.list_peers, int((info.total_size() / 1000000) * status.progress),
           info.total_size() / 1000000, int(status.progress * 100), status.download_rate / 1000, status.upload_rate / 1000);

    if (midnight())
        main->lastAskTime = NULL;
    if (main->lastAskTime != NULL)
        printw("Last ask - piece №%d, %ss ago\n", main->lastAsk, (QTime::currentTime() - *main->lastAskTime).toString("hh:mm:ss").toLocal8Bit().constData());
    std::vector<partial_piece_info> inf;
    main->torrent->get_download_queue(inf);
    if (inf.size() > 0)
        for (unsigned int i = 0; i < inf.size(); i++)
            printw("(%d, speed-%d) ", inf[i].piece_index, inf[i].piece_state);
    refresh();\
}

void MainWindow::die(QString error) {
    qDebug() << error;
    exit(1);
}

void MainWindow::setupTimers() {
    QTimer *keysTimer = new QTimer;
    keysTimer->setInterval(10);
    QObject::connect(keysTimer, SIGNAL(timeout()), this, SLOT(checkKeys()));
    keysTimer->start();

    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateStandartText()));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();
}

void MainWindow::checkKeys() {
    int key = wgetch(stdscr);
    if (key == ERR)
        return;

    if (key == '+')
        session->set_download_rate_limit(session->download_rate_limit() + 10000);
    else if (key == '-')
        session->set_download_rate_limit(session->download_rate_limit() - 10000);

    updateStandartText();
    updateInform();
}

bool MainWindow::midnight() {
    return (QTime::currentTime().hour() == 0) && (QTime::currentTime().minute() == 0) &&
            (QTime::currentTime().second() > 0) && (QTime::currentTime().second() < 5);
}
