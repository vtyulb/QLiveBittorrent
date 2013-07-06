#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QTime>
#include <QString>
#include <QFile>
#include <QVector>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>

using libtorrent::torrent_status;

#ifdef PORTABLE
    const QString settingsPath = "/.qlivebittorrent/";
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

//What a beautifull bug
inline QString getNormalStatus(libtorrent::torrent_status::state_t status) {
    if (status == torrent_status::downloading)
        return "downloading";
    else if (status == torrent_status::finished)
        return "finished";
    else if (status == torrent_status::seeding)
        return "seeding";
    else if (status == torrent_status::checking_resume_data)
        return "checking_resume_data";
    else if (status == torrent_status::checking_files)
        return "checking_files";
    else
        return "Calculating first 1000000000! digits of PI\n";
}

inline QString setStringSize(QString s, int size) {
    if (s.length() > size)
        s = s.left(size);

    int len = s.length();
    for (int i = 0; i < (size - len + 1) / 2; i++)
        s = ' ' + s;

    for (int i = 0; i < (size - len) / 2; i++)
        s = s + ' ';
    return s;
}

inline QByteArray saveResumeData(const libtorrent::save_resume_data_alert *rd) {
    QVector<char> data;
    bencode(std::back_inserter(data), *rd->resume_data);

    QByteArray res(data.size(), 0);
    for (int i = 0; i < res.size(); i++)
        res[i] = data[i];
    return res;
}

inline bool isMagnet(QString s) {
    return s.left(8) == QString("magnet:?");
}

#endif // CONSTANTS_H
