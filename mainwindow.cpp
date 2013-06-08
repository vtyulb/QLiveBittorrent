#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>

#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include <boost/filesystem/operations.hpp>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
