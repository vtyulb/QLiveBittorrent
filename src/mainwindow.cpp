#include "mainwindow.h"

MainWindow::MainWindow(QString torrent, QString downloadPath, QString mountPath, QString rate, bool gui, QObject *parent): QObject(parent) {
    informationFlushed = false;
    initSession(rate);
    if (!gui) {
        initscr();
        nodelay(stdscr, true);
        noecho();
    }

    main = NULL;
    fake = NULL;
    if (!gui) {
        realAddTorrent(torrent, downloadPath, mountPath);
        setupTimers();
    } else {
        fake = new QMainWindow;
        ui = new Ui_MainWindow;
        ui->setupUi(fake);
        QObject::connect(ui->actionAdd_torrent, SIGNAL(triggered()), this, SLOT(addTorrent()));
        QObject::connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
        QStringList labels;
        labels << "name" << "status" << "progress" << "seeds" << "connected" << "dspeed (KB/s)" << "uspeed (KB/s)" << "size (MB)";
        ui->tableWidget->setColumnCount(8);
        ui->tableWidget->setHorizontalHeaderLabels(labels);
        ui->frame->hide();
        QTimer *timer = new QTimer;
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateTable()));
        QObject::connect(ui->remountButton, SIGNAL(clicked()), this, SLOT(remountRequest()));
        timer->setInterval(1000);
        timer->start();
        findTorrents();
        QObject::connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(updateTable()));
        QObject::connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(reCell()));
        QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
        QObject::connect(ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));
        fake->show();
    }
}

MainWindow::~MainWindow() {
    if (hasGUI())
        fake->hide();
    else
        endwin();

    if ((main == NULL) || (!main->torrent->is_valid() || (main->torrent->status().progress < 0.5))) {
        informationFlushed = true;
        qDebug() << "Sending information about (upload/download) sizes to tracker";
        delete session;
        return;
    }
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
        s.setValue("path", QVariant(mapTorrent[v[i].name()]->downloadPath));
        s.sync();
    }

    informationFlushed = true;
    qDebug() << "Sending information about (upload/download) sizes to trackers";
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
    else if (!hasGUI())
        die("User don't choose torrent file");
}

void MainWindow::findPaths(QString torrent) {
    TorrentDialog *dialog = new TorrentDialog(torrent, fake);
    dialog->show();
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), this, SLOT(realAddTorrent(QString, QString, QString)));
    QObject::connect(dialog, SIGNAL(rejected()), qApp, SLOT(quit()));
}

void MainWindow::updateStandartText() {
    if (hasGUI())
        return;

    QString text;
    text += QString("Mount status: ");
    if (main->mountStatus())
        text += "mounted\n";
    else
        text += "error ('r' to remount)\n";

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
    if (!QFile::exists(torrentFile) && !isMagnet(torrentFile)) {
        if (!hasGUI())
            die("torrent file not found");\
        else
            return;
    }

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

        url = url.replace("%3A", "MyNameIsVladislavTyulbashev");
        QUrl name = QUrl::fromEncoded(url.toLocal8Bit());
        qDebug() << "Magnet link is valid:" << name.isValid();
        if (!name.isValid())
            qDebug() << name.errorString();
        url = name.toDisplayString().replace('+', ' ').replace("MyNameIsVladislavTyulbashev", ":");

        p.save_path = (torrentPath + url + "/").toStdString();

        const torrent_handle h = libtorrent::add_magnet_uri(*session, torrentFile.toStdString(), p);
        main = new Torrent(torrentPath + url, mountPath + url, h, this);
        mapTorrent[main->torrent->name()] = main;
    } else {
        torrent_info *inf = new libtorrent::torrent_info(torrentFile.toStdString());
        p.ti = inf;
        p.save_path = (torrentPath + QString::fromStdString(inf->name()) + "/").toStdString();
        main = new Torrent(torrentPath + QString::fromStdString(inf->name()), mountPath + QString::fromStdString(inf->name()), session->add_torrent(p), this);
        mapTorrent[main->torrent->name()] = main;
    }
}

void MainWindow::updateInform() {
    if (hasGUI())
        return;

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
        printw("Last ask - piece №%d, %ss ago", main->lastAsk, (QTime::currentTime() - *main->lastAskTime).toString("HH:mm:ss").toLocal8Bit().constData());
    std::vector<partial_piece_info> inf;
    main->torrent->get_download_queue(inf);
    if (inf.size() > 0)
        for (unsigned int i = 0; i < inf.size(); i++) {
            if (i % 5 == 0)
                printw("\n");

            printw("(%d, speed-%d) ", inf[i].piece_index, inf[i].piece_state);
        }
    refresh();
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
    else if (key == 'r')
        main->remount();

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

void MainWindow::updateTable() {
    std::vector<torrent_handle> torrents = session->get_torrents();
    ui->tableWidget->setRowCount(torrents.size());
    for (int i = 0; i < torrents.size(); i++) {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(torrents[i].name())));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(getNormalStatus(torrents[i].status().state)));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(torrents[i].status().progress )));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(torrents[i].status().list_seeds)));
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(torrents[i].status().num_connections)));
        ui->tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(torrents[i].status().download_rate / 1000)));
        ui->tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(torrents[i].status().upload_rate / 1000)));
        ui->tableWidget->setItem(i, 7, new QTableWidgetItem(QString::number(torrents[i].get_torrent_info().total_size() / 1000000)));
    }

    if (torrents.size()) {
//        ui->widget = new GenerateImage(torrents[ui->tableWidget->currentRow()]);
//        ui->widget->repaint();
        torrent_handle *active = &torrents[ui->tableWidget->currentRow()];
        if ((ui->tableWidget->currentRow() < 0) || (ui->tableWidget->currentRow() >= torrents.size())) {
            ui->frame->hide();
            return;
        }
        ui->frame->show();

        ui->downloadPathLabel->setText("Download path: " + mapTorrent[active->name()]->downloadPath);
        ui->mountLabel->setText("Mounted to: " + mapTorrent[active->name()]->mountPath);
        if (mapTorrent[active->name()]->mountStatus())
            ui->mountLabel->setText(ui->mountLabel->text() + "(OK)");
        else
            ui->mountLabel->setText(ui->mountLabel->text() + "(ERROR)");
    } else
        ui->frame->hide();

    fake->update();
}

bool MainWindow::hasGUI() {
    return fake != NULL;
}

void MainWindow::realRemount(QString mount) {
    mapTorrent[session->get_torrents()[ui->tableWidget->currentRow()].name()]->setMountPath(mount);
    mapTorrent[session->get_torrents()[ui->tableWidget->currentRow()].name()]->remount();
}

void MainWindow::remountRequest() {
    if (ui->tableWidget->rowCount()) {
        SetMountPathDialog *dialog = new SetMountPathDialog(mapTorrent[session->get_torrents()[ui->tableWidget->currentRow()].name()]->mountPath);
        dialog->show();
        QObject::connect(dialog, SIGNAL(accepted(QString)), this, SLOT(realRemount(QString)));
    }
}

void MainWindow::findTorrents() {
    QDir dir(settingsPath);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); i++)
        if (!s.contains(list[i].fileName()))
            if (list[i].fileName().right(16) == QString(".qlivebittorrent")) {
                addTorrentByName(list[i].fileName());
                s.insert(list[i].fileName());
            }
}

void MainWindow::addTorrentByName(QString torrent) {
    QSettings s(settingsPath + torrent, QSettings::IniFormat);
    QByteArray data = s.value("data").toByteArray();
    libtorrent::entry e = libtorrent::bdecode(data.begin(), data.end());
    libtorrent::torrent_info *inf = new libtorrent::torrent_info(e);
    const libtorrent::torrent_handle h =
            session->add_torrent(inf, (s.value("path").toString()).toStdString(), e);

    h.set_upload_mode(true);
    h.auto_managed(false);
    if (h.is_paused())
        h.resume();

    mapTorrent[h.name()] = new Torrent(s.value("path").toString(), "/", h, this);
}

void MainWindow::reCell() {
    ui->tableWidget->setRangeSelected(QTableWidgetSelectionRange(ui->tableWidget->currentRow(), 0, ui->tableWidget->currentRow(), 7), true);
    fake->repaint();
}

void MainWindow::showAbout() {
    QMessageBox::about(fake, "About QLiveBittorrent", "QLiveBittorrent - opensource program, which use Qt and libtorrent-rasterbar.\nWritten by V.S.Tyulbashev.\n<vladislav.tyulbashev@yandex.ru>");
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(fake);
}
