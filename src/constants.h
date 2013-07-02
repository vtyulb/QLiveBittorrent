#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QTime>

#ifdef PORTABLE
    const QString settingsPath = "./";
    const QString driver="./qlivebittorrent-driver";
#else
    const QString settingsPath = QDir::homePath() + "/.qlivebittorrent/";
    const QString driver="qlivebittorrent-driver";
#endif

inline QTime operator -(const QTime &a, const QTime &b) {
    int seconds = a.second() + a.minute() * 60 + a.hour() * 3600;
    seconds -= b.second() + b.minute() * 60 + b.hour() * 3600;
    return QTime(seconds / 3600, seconds % 3600 / 60, seconds % 60);
}



#endif // CONSTANTS_H
