#include "mainwindow.h"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <QCoreApplication>
#include <QDebug>

namespace po=boost::program_options;

void showHelp() {
    printf("Usage: qlivebittorrent [-t string] [-d string] [-m string] [-r int] [-h] [-g]\n");
    printf("\t-t --torrent string      - name of *.torrent file\n");
    printf("\t-d --downloadpath string - name of folder to download files\n");
    printf("\t-m --mount string        - name of folder to mount files from torrent\n");
    printf("\t-g --gui                 - to strart a little GUI Interface\n");
    printf("\t-r --limit-rate int      - to limit downloading rate (in KB)\n");
    printf("\t-h --help                - to view this help\n");

    exit(0);
}

int main(int argc, char *argv[])
{
    std::string torrent;
    std::string downloadPath;
    std::string mountPath;
    std::string rate;

    po::options_description desc("General options");
    desc.add_options()
            ("torrent,t", po::value<std::string>(&torrent), "Input torrent file")
            ("downloadpath,d", po::value<std::string>(&downloadPath), "Download path")
            ("mount,m", po::value<std::string>(&mountPath), "Path to mount files")
            ("limit-rate,r", po::value<std::string>(&rate), "Limit rate")
            ("gui,g", "Start a little gui")
            ("help,h", "Show help");

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help"))
        showHelp();

    qDebug() << QString::fromStdString(torrent) << QString::fromStdString(downloadPath) << QString::fromStdString(mountPath);
    if (vm.count("gui")) {
        QApplication a(argc, argv);
        MainWindow w(QString::fromStdString(torrent), QString::fromStdString(downloadPath), QString::fromStdString(mountPath), QString::fromStdString(rate), vm.count("gui"));
        return a.exec();
    } else {
        QCoreApplication a(argc, argv);
        MainWindow w(QString::fromStdString(torrent), QString::fromStdString(downloadPath), QString::fromStdString(mountPath), QString::fromStdString(rate), vm.count("gui"));
        return a.exec();
    }
}
