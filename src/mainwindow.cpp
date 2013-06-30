#include "mainwindow.h"

MainWindow::MainWindow(QString torrent, QString downloadPath, QString mountPath, QString rate, bool gui, QObject *parent): QObject(parent) {
    initSession(rate);
    if (!gui)
        realAddTorrent(torrent, downloadPath, mountPath);
    else {
        fake = new QMainWindow;
        if (QFile(torrent).exists())
            findPaths(torrent);
        else
            addTorrent();

        fake->deleteLater();
    }

    initscr();
}

MainWindow::~MainWindow() {
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

void MainWindow::realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath) {
    if (!QFile::exists(torrentFile))
        die("torrent file not found");

    standartText = new QByteArray;
    *standartText = ("Torrent file: " + torrentFile + "\nDownload path: " + torrentPath + "\nMount path: " + mountPath + "\n" +
               "==================================================================\n").toLocal8Bit();

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

    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();
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
    printw("%s", standartText->constData());
    printw("%d of %d peers connected; %d of %d MB downloaded; speed - %dKB/s\n",
           status.num_connections, status.list_seeds, status.total_payload_download / 1000000, info.total_size() / 1000000, status.download_rate / 1000);
    printw("Last ask - %d piece\n", main->lastAsk);
    std::vector<partial_piece_info> inf;
    main->torrent->get_download_queue(inf);
    if (inf.size() > 0)
        for (unsigned int i = 0; i < inf.size(); i++)
            printf("(%d, speed-%d) ", inf[i].piece_index, inf[i].piece_state);
    refresh();\
}

void MainWindow::die(QString error) {
    qDebug() << error;
    exit(1);
}
