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
    updateInform();
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateInform()));
    timer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initSession() {
    session = new libtorrent::session;
//    libtorrent::session_settings settings = session->settings();
//    settings.`
    session->listen_on(std::make_pair(6881, 6889));
}

void MainWindow::addTorrent() {
    QString torrent = QFileDialog::getOpenFileName(this, QString(), QString(),
                                                   QString("*.torrent"));
    libtorrent::add_torrent_params p;
    p.save_path = "/home/vlad/test/";
    libtorrent::torrent_info *inf = new libtorrent::torrent_info(torrent.toStdString());
    p.ti = inf;

    new Torrent(mountPath + QString::fromStdString(inf->name()), session->add_torrent(p), this);
}

void MainWindow::updateInform() {
    std::vector<libtorrent::torrent_handle> v = session->get_torrents();

    QStringList list;
    list << "Name" << "Size" << "Downloaded" << "Download rate" << "Seeds" << "Connected";
    QStandardItemModel *model = new QStandardItemModel;
    model->setColumnCount(6);
    model->setRowCount(v.size());
    model->setHorizontalHeaderLabels(list);
    for (int i = 0; i < v.size(); i++) {
        libtorrent::torrent_status s = v[i].status();
        libtorrent::torrent_info inf = v[i].get_torrent_info();
        model->setItem(i, 0, new QStandardItem(QString::fromStdString(v[i].name())));
        model->setItem(i, 1, new QStandardItem(QString::number(inf.total_size())));
        model->setItem(i, 2, new QStandardItem(QString::number(s.total_download)));
        model->setItem(i, 3, new QStandardItem(QString::number(s.download_rate)));
        model->setItem(i, 4, new QStandardItem(QString::number(s.num_seeds)));
        model->setItem(i, 5, new QStandardItem(QString::number(s.num_connections)));
    }

    QAbstractItemModel *m = ui->tableView->model();
    ui->tableView->setModel(model);
    delete m;
}
