Navi: Directory based music player
==================================

Navi is my attempt to create a user friendly, easy to use and featureful music
player for Linux. There are already plenty of good music players available for
Linux, for instance:

* [Banshee](http://banshee.fm) - using C# (Mono), GTK+, Gstreamer
* [Rhythmbox](http://projects.gnome.org/rhythmbox/) - C, GTK, Gstreamer
* [Listen](http://www.listen-project.org/) - Python, GTK
* [xmms](http://www.xmms.org) - Winamp-like
* [Amarok](http://amarok.kde.org) - Qt(C++)
* [Exaile](http://exaile.org) - Python, influenced by Amarok, GStreamer

These players all have lots of features and extensibility, but what I was 
missing most in those players, was simple directory based music playing. My 
music collection is mostly organized by the use of directories:

    Albums
        Boemklatsch
            Spontaneous Combustion
                CD 1
                CD 2
        Dr. Lektroluv
            Live at Lowlands 2010
        Hed Kandi
            Hed Kandi Ibiza 2010
                CD 1
                CD 2
                CD 3
    Dance
        (400 files)
    Game OSTs
        Bayonetta
        Mirror's Edge
        Dragon Age

I usually just play everything which is in one directory only. There was no 
real directory-based music player available for Linux (AFAIK...), and therefore 
I decided to just write my own. It also provided me a good reason to learn
[GStreamer](http://gstreamer.net) and [wxWidgets](http://wxwidgets.org).

Current features
----------------

This is a list with the current implemented features:

* The obvious: play, pause, stop, seek, next and previous track;
* Directory based media browser;
* Internet radio stations (streaming audio). Can be added and removed, and are
persisted to disk;
* Reading tags from streams and files;
* 'System tray' icon, for less display hassle in the window list in your
Desktop environment (may have a buggy display);

Planned features
----------------
The following features are planned or work in progress:

* Caching system. If you have a large directory, re-reading all tags can be VERY
tedious and time consuming. A sort of proxy is needed here;
* Preferences window (user preferences);
* Randomize the current directory-playlist;
* Favorites, play queue or the like is a must too;
* I'm also thinking of a full recursive 'playlist', i.e. using the base directory,
find all media files within all the subdirectories. This will REALLY need caching :)
* OSD (On Screen Display) notification, to get notified which track is currently
playing. This used to be implemented, but the prototypes in ``libnotify`` are not
compatible amongst different versions :(

Technological information 
-------------------------

For those interested, I am building the software with the following tools and 
libraries (aka dependencies)

* C++ programming language;
* [wxWidgets](http://wxwidgets.org) 2.8.10+ for UI elements and the like;
* The excellent [GStreamer](http://gstreamer.net) 0.10.30+ as audio backend;

Build instructions
-------------------------
Navi is still in an alpha stage, so no pre-packaged version is available. You can
however build the sources yourself. To do this, either download the zip file of the
master branch [here](https://github.com/krpors/navi/zipball/master), or clone the Git
repository:

    git clone git://github.com/krpors/navi.git

Since it depends on wxWidgets, gstreamer and libnotify, you'll need to install those
dependencies as well. For Debian based systems, you can invoke the following command:

    sudo apt-get install build-essential libwxgtk2.8-dev libgstreamer0.10-dev

After installation of these packages, just run ``make`` on the ``Makefile``:

    make

And the binary will be built under the ``./bin/`` directory inside the Navi git repo.
Run it with ``./bin/navi``, or else you'll get a warning about missing icons.

Feedback
--------

If you are a user of Navi, you like or dislike what it is or where it's going to, please
let me know! I'm fond of knowing if my software is being used. Just drop a small line.

Code critique are certainly welcome too!
