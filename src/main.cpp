#include "mainwindow.h"
#include "seedmanager.h"

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <signal.h>

#include <QCoreApplication>
#include <QApplication>
#include <QDebug>

namespace po=boost::program_options;

AbstractSeedingClass *c;

void showHelp() {
    printf("Usage: qlivebittorrent (-t string) (-d string) [-m string] [-r int]\n");
    printf("   or: qlivebittorrent [-t string] (-g | --gui) [-r | --limit-rate int]\n");
    printf("   or: qlivebittorrent -s | --seeding-manager\n");
    printf("   or: qlivebittorrent (torrent | magnet)");
    printf("\t-t --torrent string      - name of *.torrent file or magnet link\n");
    printf("\t-d --downloadpath string - name of folder to download files\n");
    printf("\t-m --mount string        - name of folder to mount files from torrent\n");
    printf("\t-g --gui                 - to strart a little GUI Interface\n");
    printf("\t-r --limit-rate int      - to limit downloading rate (in KB)\n");
    printf("\t-s --seeding-manager     - to start seed-manager instead of client\n");
    printf("\t-h --help                - to view this help\n\n");
    printf("In interactive mode there are several available keys:\n");
    printf("\t'+' - For increasing download rate limit(+10KB)\n");
    printf("\t'-' - For decreasing download rate limit(-10KB)\n");
    printf("\t'q' - For quit\n\n");
    printf("\t'a' - For disable/enable agressive piece picking\n");
    printf("For example: qlivebittorrent -t example.torrent -d downloadsdir -m mountdir -r 200\n");

    exit(0);
}

void sigtermListened(int sig) {
    if (c->informationSaved()) {
        qDebug() << "force quit";
        exit(0);
    } else
        qApp->quit();
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, sigtermListened);
    signal(SIGINT, sigtermListened);

    std::string torrent, bt;
    std::string downloadPath;
    std::string mountPath;
    std::string rate;

    if ((argc == 2) && (argv[1][0] != '-')) {
        bt = argv[1];
        argv[1] = "-g";
    }

    po::options_description desc("General options");
    desc.add_options()
            ("torrent,t", po::value<std::string>(&torrent), "Input torrent file")
            ("downloadpath,d", po::value<std::string>(&downloadPath), "Download path")
            ("mount,m", po::value<std::string>(&mountPath), "Path to mount files")
            ("limit-rate,r", po::value<std::string>(&rate), "Limit rate")
            ("seeding-manager,s", "To start a seeding manager")
            ("gui,g", "Start a little gui")
            ("help,h", "Show help");

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help"))
        showHelp();


    bool magnet = isMagnet(QString::fromStdString(torrent));

    if (vm.count("seeding-manager")) {
        QCoreApplication a(argc, argv);
        SeedManager manager(&a);
        c = &manager;
        return a.exec();
    } else if (vm.count("gui") || (!QFile::exists(QString::fromStdString(torrent)) && !magnet)) {
        if ((vm.count("gui") == 0) && (!QFile::exists(QString::fromStdString(torrent))))
            printf("You must specify a valid torrent file (or --gui)\n");

        QApplication a(argc, argv);
        MainWindow w(QString::fromStdString(torrent), QString::fromStdString(downloadPath), QString::fromStdString(mountPath), QString::fromStdString(rate), true, &a);
        c = &w;
        return a.exec();
    } else {
        QCoreApplication a(argc, argv);
        MainWindow w(QString::fromStdString(torrent), QString::fromStdString(downloadPath), QString::fromStdString(mountPath), QString::fromStdString(rate), false, &a);
        c = &w;
        return a.exec();
    }
}
