#include "mainwindow.h"

MainWindow::MainWindow(QString torrent, QString downloadPath, QString mountPath, bool gui, QObject *parent): QObject(parent) {
    initSession();
    loadSettings();
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

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::initSession() {
    session = new libtorrent::session;
    libtorrent::session_settings settings = session->settings();
    settings.max_allowed_in_request_queue = 4;
    settings.seed_choking_algorithm = settings.fastest_upload;
    settings.choking_algorithm = settings.bittyrant_choker;
    session->set_settings(settings);
}

void MainWindow::addTorrent() {
    QString torrent = QFileDialog::getOpenFileName(fake, QString(), QString(),
                                                   QString("*.torrent"));
    findPaths(torrent);
}

void MainWindow::findPaths(QString torrent) {
    TorrentDialog *dialog = new TorrentDialog(torrent, fake);
    dialog->show();
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), this, SLOT(realAddTorrent(QString, QString, QString)));
}

void MainWindow::realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath) {
    if (!QFile::exists(torrentFile))
        die("torrent file not found");

    if (torrentPath[torrentPath.length() - 1] != QChar('/'))
        torrentPath += "/";
    if (mountPath[mountPath.length() - 1] != QChar('/'))
        mountPath += "/";
    add_torrent_params p;
    torrent_info *inf = new libtorrent::torrent_info(torrentFile.toStdString());
    p.save_path = (torrentPath + QString::fromStdString(inf->name()) + "/").toStdString();
    p.ti = inf;
    p.storage_mode = libtorrent::storage_mode_allocate;

    new Torrent(torrentPath + QString::fromStdString(inf->name()), mountPath + QString::fromStdString(inf->name()), session->add_torrent(p), this);
}

void MainWindow::updateInform() {
    std::vector<torrent_handle> v = session->get_torrents();

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
    }
}

void MainWindow::saveSettings() {
    QSettings s(settingsFile, QSettings::IniFormat);
    std::vector<torrent_handle> torrents = session->get_torrents();
    for (unsigned int i = 0; i < torrents.size(); i++)
        torrents[i].save_resume_data();
    Torrent::sleep(10000);
    libtorrent::entry e;
    session->save_state(e);
    //this code is copy-pasted from qbittorrent :-)
    std::vector<char> out;
    bencode(back_inserter(out), e);
    QByteArray ar;
    ar.resize(out.size());
    for (unsigned int i = 0; i < out.size(); i++)
        ar[i] = out[i];

    s.setValue("session", QVariant(ar));
}

void MainWindow::loadSettings() {
    QSettings s(settingsFile, QSettings::IniFormat);
    QByteArray ar = s.value("session").toByteArray();
    std::vector<char> in;
    in.resize(ar.size());
    for (int i = 0; i < ar.size(); i++)
        in[i] = ar[i];

    libtorrent::lazy_entry e;
    libtorrent::lazy_bdecode(&in[0], &in[in.size()], e);
    session->load_state(e);
}

void MainWindow::die(QString error) {
    qDebug() << error;
    exit(1);
}
