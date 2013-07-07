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
        if (QFile(torrent).exists() || isMagnet(torrent))
            findPaths(torrent);
        else
            addTorrent();
    }
}

MainWindow::~MainWindow() {
    endwin();
    if ((main == NULL) || (!main->torrent->is_valid() || (main->torrent->status().progress < 0.5))) {
        informationFlushed = true;
        qDebug() << "Sending information about (upload/download) sizes to tracker";
        delete session;
        return;
    }
    qDebug() << "saving information about torrent";
    std::deque<alert *> trash;
    session->pop_alerts(&trash);
    main->torrent->save_resume_data(torrent_handle::save_info_dict);
    const alert *a = session->wait_for_alert(libtorrent::seconds(3));
    if (a == NULL) {
        qDebug() << "Can not save resume data";
        exit(1);
    }

    std::auto_ptr<alert> holder = session->pop_alert();
    if (libtorrent::alert_cast<libtorrent::save_resume_data_failed_alert>(a)) {
        qDebug() << "Failed alert";
        exit(1);
    }

    const libtorrent::save_resume_data_alert *rd = libtorrent::alert_cast<libtorrent::save_resume_data_alert>(a);
    if (rd == 0) {
        qDebug() << "Very big fail";
        exit(1);
    }

    QSettings s(settingsPath + QString::fromStdString(main->torrent->name()) + ".qlivebittorrent", QSettings::IniFormat);
    s.setValue("path", QVariant(resumeSavePath));
    s.setValue("data", saveResumeData(rd));
    s.sync();

    informationFlushed = true;
    qDebug() << "Sending information about (upload/download) sizes to tracker";
    delete session;
}

void MainWindow::initSession(QString rate) {
    session = new libtorrent::session;
    libtorrent::session_settings settings = session->settings();
    settings.max_allowed_in_request_queue = 4;
    settings.seed_choking_algorithm = settings.fastest_upload;
    settings.choking_algorithm = settings.bittyrant_choker;
    session->set_settings(settings);
    session->set_download_rate_limit(rate.toInt() * 1000);
    session->set_alert_mask(0);
}

void MainWindow::addTorrent() {
    QString torrent = QFileDialog::getOpenFileName(fake, QString(), QString(),
                                                   QString("*.torrent"));
    if (QFile(torrent).exists() || isMagnet(torrent))
        findPaths(torrent);
    else
        die("User don't choose torrent file");
}

void MainWindow::findPaths(QString torrent) {
    TorrentDialog *dialog = new TorrentDialog(torrent, fake);
    dialog->show();
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), this, SLOT(realAddTorrent(QString, QString, QString)));
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), dialog, SLOT(deleteLater()));
    QObject::connect(dialog, SIGNAL(rejected()), qApp, SLOT(quit()));
}

void MainWindow::updateStandartText() {
    QString text;
    if (session->download_rate_limit() != 0)
        text += QString("Download rate limit: %1KB/s\n").arg(session->download_rate_limit() / 1000);

    if (main != NULL) {
        text += "Status: ";
        text += getNormalStatus(main->torrent->status().state);
        text += "\n";
        if (!main->agressive)
            text += "Non agressive piece picking\n";
    }

    for (int i = 1; i < stdscr->_maxx; i++)
        text += '=';
    text += '\n';

    standartText = standartText.left(standartTextLen) + text.toLocal8Bit();
}

void MainWindow::realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath) {
    if (!QFile::exists(torrentFile) && !isMagnet(torrentFile))
        die("torrent file not found");

    standartText = ("Torrent file: " + torrentFile + "\nDownload path: " + torrentPath + "\nMount path: " + mountPath + "\n").toLocal8Bit();
    standartTextLen = standartText.size();
    updateStandartText();

    if (torrentPath[torrentPath.length() - 1] != QChar('/'))
        torrentPath += "/";
    if (mountPath[mountPath.length() - 1] != QChar('/'))
        mountPath += "/";

    resumeSavePath = torrentPath;

    add_torrent_params p;
    p.storage_mode = libtorrent::storage_mode_allocate;

    if (isMagnet(torrentFile)) {
        int i;
        for (i = 1; torrentFile[i] != '='; i++);
        for (i = i + 1; torrentFile[i] != '='; i++);

        QString url;
        for (i = i + 1; torrentFile[i] != '&'; i++)
            url += torrentFile[i];

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QString name = QUrl(url).toString().replace("+", " ");
#else
        QString name = QUrl(url).toString(QUrl::PreferLocalFile).replace("+", " ");
#endif
        p.save_path = (torrentPath + name + "/").toStdString();

        const torrent_handle h = libtorrent::add_magnet_uri(*session, torrentFile.toStdString(), p);
        main = new Torrent(torrentPath + QString::fromStdString(h.name()), mountPath + QString::fromStdString(h.name()), h, this);
    } else {
        torrent_info *inf = new libtorrent::torrent_info(torrentFile.toStdString());
        p.ti = inf;
        p.save_path = (torrentPath + QString::fromStdString(inf->name()) + "/").toStdString();
        main = new Torrent(torrentPath + QString::fromStdString(inf->name()), mountPath + QString::fromStdString(inf->name()), session->add_torrent(p), this);
    }

    setupTimers();
}

void MainWindow::updateInform() {
    erase();
    libtorrent::torrent_status status = main->torrent->status();
    libtorrent::torrent_info info = main->torrent->get_torrent_info();
    printw("%s", standartText.constData());
    printw("%d of %d peers connected; %d of %d MB downloaded;\nprogress - %d%% d - %dKB/s; u - %dKB/s\n",
           status.num_connections, status.list_seeds + status.list_peers, int((info.total_size() / 1000000) * status.progress),
           info.total_size() / 1000000, int(status.progress * 100), status.download_rate / 1000, status.upload_rate / 1000);

    if (midnight())
        main->lastAskTime = NULL;
    if (main->lastAskTime != NULL)
        printw("Last ask - piece №%d, %ss ago\n", main->lastAsk, (QTime::currentTime() - *main->lastAskTime).toString("HH:mm:ss").toLocal8Bit().constData());
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
    else if (key == 'a')
        main->invertAgressive();

    updateStandartText();
    updateInform();
}

bool MainWindow::midnight() {
    return (QTime::currentTime().hour() == 0) && (QTime::currentTime().minute() == 0) &&
            (QTime::currentTime().second() > 0) && (QTime::currentTime().second() < 5);
}

bool MainWindow::informationSaved() {
    return informationFlushed;
}
