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
missing most in those players, was simple directory based music playing. You
see, my music collection is mostly organized by the use of directories:

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
real directory-based music player available for Linux, and therefore I 
decided to just write my own. It also provided me a good reason to learn
[GStreamer](http://gstreamer.net) and [wxWidgets](http://wxwidgets.org).

(Planned) features
------------------

The following list describes the (non-exhaustive) functionalities I plan to 
at least implement in Navi:

* GTK User Interface, with 'system tray', and OSD notifications (not sure yet
how I'm going to do that yet, but I like that);
* Playlists;
* Queueing of tracks;
* Of course the obvious: play, pause, stop, seek, volume changing;
* Album art;
* Internet radio stations (streaming audio);
* Audioscrobbler/Last.fm submitting;
* Tag reading (and possibly writing);
* 'System tray' icon, for less display hassle in the window list in your
Desktop environment.

Technological information 
-------------------------

For those interested, I am building the software with the following tools and 
libraries:

* C++ programming language;
* [wxWidgets](http://wxwidgets.org) 2.8.10+ for UI elements and the like;
* The excellent [GStreamer](http://gstreamer.net) 0.10.30+ as audio backend;

Feedback
--------

Feedback to this application is certainly welcome, like ideas, wishes, or just
some plain old critique. Also, code inspection is good too.
