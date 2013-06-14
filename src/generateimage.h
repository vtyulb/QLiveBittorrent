#ifndef GENERATEIMAGE_H
#define GENERATEIMAGE_H

#include <QRgb>
#include <QImage>
#include <libtorrent/bitfield.hpp>
#include <libtorrent/torrent_handle.hpp>

using libtorrent::bitfield;
using libtorrent::partial_piece_info;
using std::vector;

class GenerateImage
{
public:
    GenerateImage();
    static QImage generate(const bitfield &b, const vector<partial_piece_info> &v);
};

#endif // GENERATEIMAGE_H
