#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionAdd_torrent, SIGNAL(triggered()), this, SLOT(addTorrent()));

    initSession();
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(showInform()));
    timer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initSession() {
    session = new libtorrent::session;
    session->listen_on(std::make_pair(6881, 6889));
}

void MainWindow::addTorrent() {
    QString torrent = QFileDialog::getOpenFileName(this, QString(), QString(),
                                                   QString("*.torrent"));
    libtorrent::add_torrent_params p;
    p.save_path = "/home/vlad/test/";
    p.ti = new libtorrent::torrent_info(torrent.toStdString());
    session->add_torrent(p);
}

void MainWindow::showInform() {
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();
    qDebug() << v.size();
    for (int i = 0; i < v.size(); i++) {
        libtorrent::torrent_status s = v[i].status();
        qDebug() << i << s.download_rate << s.total_download;
    }
    qDebug() << "------------------------------";
}
