QLiveBittorrent
===============

Bittorrent client with function reading files before they would be downloaded

You'll need libtorrent, ncurses, boost-system, boost-program-options and qt5.
```
gcc -g driver.c `pkg-config fuse --cflags --libs` -o qlivebittorrent-driver
qmake -makefile && make
cp qlivebittorrent /usr/bin/ && cp qlivebittorrent-driver /usr/bin/
qlivebittorrent --help
```

##### ArchLinux
Just use AUR package: https://aur.archlinux.org/packages/qlivebittorrent-git

##### Ubuntu 16.10+ && Debian testing
Use packages provided by [DAP-DarkneSS](https://github.com/DAP-DarkneSS)  
Or make project manually:
```
apt-get install ncurses-dev libtorrent-rasterbar-dev g++ make qt5-default libboost-system-dev libboost-program-options-dev
gcc -g driver.c `pkg-config fuse --cflags --libs` -o qlivebittorrent-driver
qmake -makefile && make
cp qlivebittorrent /usr/bin/ && cp qlivebittorrent-driver /usr/bin/
```


##### Other information
qlivebittorrent try to find qlivebittorrent-driver in the working directory.
In non-Portable version (define PORTABLE in qlivebittorrent.pro) it would try
to find it in PATH

GUI interface is not done, so please don't try to use it :-)

Current status of program:
I tested it on archlinux, by downloading 1.45G file on 3Mbit channel - it works good.
If you want to test it, you should understand that my program is not a god, so piece,
which you want to see can be downloaded with a big delay.

You can find more information (russian language) at:
https://habrahabr.ru/post/185770/

Plus and Minus keys - to increase/decrease download rate limit.
'a' - to switch agressive/non-agressive piece picking;

GPLv2+ license (see COPYING for details)
