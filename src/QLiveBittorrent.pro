#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:11:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QLiveBittorrent
TEMPLATE = app

LIBS += -L/usr/lib -lboost_system -lboost_program_options -lboost_thread  -lncurses -lpthread -ltorrent-rasterbar

SOURCES += main.cpp\
        mainwindow.cpp \
    torrent.cpp \
    generateimage.cpp \
    torrentdialog.cpp \
    getfolderdialog.cpp \
    seedmanager.cpp \
    abstractseedingclass.cpp \
    setmountpathdialog.cpp

HEADERS  += mainwindow.h \
    torrent.h \
    generateimage.h \
    torrentdialog.h \
    getfolderdialog.h \
    seedmanager.h \
    constants.h \
    abstractseedingclass.h \
    setmountpathdialog.h

FORMS    += \
    torrentdialog.ui \
    mainwindow.ui \
    setmountpathdialog.ui

PKGCONFIG += libtorrent-rasterbar

DEFINES += TORRENT_USE_OPENSSL BOOST_ASIO_HASH_MAP_BUCKETS=1021 BOOST_EXCEPTION_DISABLE BOOST_ASIO_ENABLE_CANCELIO TORRENT_LINKING_SHARED
#DEFINES += PORTABLE
