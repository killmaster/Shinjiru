# Shinjiru #

## Description ##

Shinjiru is anime list management tool built with Qt designed to automatically update your progress to AniList and act an desktop manager for AniList.

## Installation ##

Shinjiru's website currently hosts win32 alpha builds [here](http://app.shinjiru.me/latest.php).  
For other build needs feel free to [build](#Building) Shinjiru for yourself.


## Building ##

**Dependencies**

Shinjiru requires a few dependencies to compile and work properly, fortunately they are relatively easy to setup.

- **Qt**         - [http://www.qt.io/download-open-source/](http://www.qt.io/download-open-source/)
- **OpenSSL**    - [http://www.openssl.org/](http://www.openssl.org/)

The following dependencies should be cloned and place into the lib folder.

- **QtAwesome**  - [https://github.com/matt4682/QtAwesome](https://github.com/matt4682/QtAwesome)
- **AniListAPI** - [https://github.com/matt4682/AniListAPI](https://github.com/matt4682/AniListAPI)
- **Anitomy**    - [https://github.com/erengy/anitomy](https://github.com/erengy/anitomy)
- **Fervor**     - [https://github.com/pypt/fervor](https://github.com/pypt/fervor)

**app.h**

The file `app.h` contains two const QStrings, `api_id` and `api_secret`.  
Set the two aforementioned variables to your own client id and client secret if you are building it yourself, mine belong to me. :)

If you are using your own api information you should also change the update URL and authorization page URL.

## Contribute ##

If you want to contribute to Shinjiru feel free to create a pull request or an issue with suggestions/issues/features etc.

Make sure your code compiles if you're contributing.  
I don't have insanely high standards for code style but try to keep the code organized.

## Upcoming ##

Here are some things I plan to do in no real order.

+ <s>Actually update lists from the AnimePanel</s>
+ <s>Implement the statistics page</s> *
	+ Duration is in large anime models only
	+ Mean / Median / Mode / Deviation
	+ Shinjiru stats (uptime, torrents dl'd since launch, minutes watched during session etc)
+ <s>Implement the airing page</s> *
	+ Sometimes airing anime don't have their cover art properly downloaded
	+ Double-click to open up details pane
	+ Filter to watching anime? Sort into list categories?
+ <s>Implement auto downloading of torrents as rules</s>
+ Add a rule management pane
+ Add expiry dates to rules to keep things clean
+ <s>Implement anime recognition</s> *
+ <s>Add some actual settings</s> *
+ Add help pane
+ <s>Add about pane</s>
+ <s>Add updater</s>
+ Better support for custom lists
+ Better support for Japanese and English titles
+ <s>Add an icon</s>
+ <s>Minimize to tray / Close to tray</s> *
+ <s>Fix the weird menubar bug</s>
+ Probably some other stuff I'm forgetting.

Starred items require improvement but basic functionality should exist.

## Contact ##

- **Email**: <admin@shinjiru.me>
- **Twitter**: [https://twitter.com/Kazakuri](https://twitter.com/Kazakuri)
- **Website**: [http://shinjiru.me](http://shinjiru.me)
- **GitHub**: [https://github.com/matt4682/AniListAPI](https://github.com/matt4682/AniListAPI)