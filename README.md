# Shinjiru #

## Description ##

Shinjiru is anime list management tool built with Qt designed to automatically update your progress to AniList and act an desktop manager for AniList.

## Installation ##

<strike>Shinjiru's [official site](http://app.shinjiru.me/) currently offers an installation package for Windows x86 (Qt 5.3 MinGW 32bit). For other build needs feel free to [build](#Building) Shinjiru for yourself.</strike>

Just kidding, if you want a build, build it yourself or contact me.

## Building ##

**Dependencies**

Shinjiru requires a few dependencies to compile and work properly, fortunately they are relatively easy to setup.

- **Qt** - [http://www.qt.io/download-open-source/](http://www.qt.io/download-open-source/)
- **QtAwesome** - [https://github.com/matt4682/QtAwesome](https://github.com/matt4682/QtAwesome)
- **AniListAPI** - [https://github.com/matt4682/AniListAPI](https://github.com/matt4682/AniListAPI)
- **Anitomy** - [https://github.com/erengy/anitomy](https://github.com/erengy/anitomy)

**app.h**

The file `app.h` contains two const QStrings, `api_id` and `api_secret`.  
Set the two aforementioned variables to your own client id and client secret if you are building it yourself, mine belong to me. :)

## Contribute ##

We'll talk about this later.

## Upcoming ##

Here are some things I plan to do in no real order.

+ <s>Actually update lists from the AnimePanel</s>
+ Implement the statistics page
+ Implement the airing page
+ Implement auto downloading of torrents as rules
+ Implement anime recognition
+ Add some actual settings
+ Add help pane
+ Add about pane
+ Add updater
+ Better support for custom lists
+ Better support for Japanese and English titles
+ Add an icon
+ Minimize to tray / Close to tray
+ Fix the weird menubar bug
+ Probably some other stuff I'm forgetting.

## Contact ##

- **Email**: <admin@shinjiru.me>
- **Twitter**: [https://twitter.com/Kazakuri](https://twitter.com/Kazakuri)
- **Website**: [http://shinjiru.me](http://shinjiru.me)
- **GitHub**: [https://github.com/matt4682/AniListAPI](https://github.com/matt4682/AniListAPI)