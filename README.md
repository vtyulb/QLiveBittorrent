QLiveBittorrent
===============


Bittorrent client with function reading files before they would be downloaded
Compile driver.c with string: gcc -g `pkg-config fuse --cflags --libs` driver.c -o driver

TODO:
http protocol (maybe)

Current status of program:
I tested it on archlinux, by downloading 1.45G file on 8Mbit channel - it works good.
If you want to test it, you should understand that my program is not a god, so piece,
which you want to see can be downloaded with a big delay.

Plus and Minus keys - to increase/decrease download rate limit.

GPL License
