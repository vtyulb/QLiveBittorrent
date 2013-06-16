#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionAdd_torrent, SIGNAL(triggered()), this, SLOT(addTorrent()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));

    initSession();
    loadSettings();
    initTableWidgetHeader();
    updateInform();
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
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
    QString torrent = QFileDialog::getOpenFileName(this, QString(), QString(),
                                                   QString("*.torrent"));
    TorrentDialog *dialog = new TorrentDialog(torrent, this);
    dialog->show();
    QObject::connect(dialog, SIGNAL(success(QString,QString,QString)), this, SLOT(realAddTorrent(QString, QString, QString)));
}

void MainWindow::realAddTorrent(QString torrentFile, QString torrentPath, QString mountPath) {
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

void MainWindow::initTableWidgetHeader() {
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QString("Name")));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(QString("Size")));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(QString("Status")));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(QString("Speed")));
    ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(QString("Seeds")));
    ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(QString("Connected")));
}

void MainWindow::updateInform() {
    std::vector<torrent_handle> v = session->get_torrents();

    ui->tableWidget->setRowCount(v.size());
    for (unsigned int i = 0; i < v.size(); i++) {
        torrent_status s = v[i].status();
        torrent_info inf = v[i].get_torrent_info();
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(v[i].name())));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(inf.total_size())));
        std::vector<partial_piece_info> tmp;
        v[i].get_download_queue(tmp);
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QIcon(QPixmap::fromImage(GenerateImage::generate(s.pieces, tmp))), QString()));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(s.download_rate)));
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(s.num_seeds)));
        ui->tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(s.num_connections)));
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

void MainWindow::showAbout() {
    QMessageBox::about(this, QString("About QLiveBittorrent"), QString("QLiveBittorrent - bittorrent client.\nWritten by Vladislav Tyulbashev.\n<vladislav.tyulbashev@yandex.ru>"));
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(this);
}
