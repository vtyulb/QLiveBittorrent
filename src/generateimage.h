#ifndef GENERATEIMAGE_H
#define GENERATEIMAGE_H

#include <QRgb>
#include <QImage>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>

#include <libtorrent/bitfield.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

using libtorrent::bitfield;
using libtorrent::partial_piece_info;
using libtorrent::torrent_handle;
using libtorrent::torrent_info;
using std::vector;

class GenerateImage: public QWidget
{
public:
    GenerateImage(const torrent_handle &h);
    static QImage generate(const bitfield &b, const vector<partial_piece_info> &v);\
protected:
    void paintEvent(QPaintEvent *);
private:
    torrent_handle torrent;
};

#endif // GENERATEIMAGE_H
