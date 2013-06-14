#include "generateimage.h"

GenerateImage::GenerateImage()
{
}

QImage GenerateImage::generate(const bitfield &b, const vector<partial_piece_info> &p) {
    QImage image(b.size(), 1, QImage::Format_RGB32);

    int w[b.size()];
    for (int i = 0; i < b.size(); i++)
        w[i] = 0;

    for (int i = 0; i < p.size(); i++)
        w[p[i].piece_index] = p[i].piece_state;

    QRgb color;
    for (int i = 0; i < b.size(); i++) {
        if (b[i])
            color = qRgb(0, 0, 255);
        else if (w[i])
            color = qRgb(0, 255, 0);
        else
            color = qRgb(255, 0, 0);

        image.setPixel(i, 0, color);
    }

    return image;
}
