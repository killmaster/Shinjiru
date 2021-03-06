# Shinjiru #

## Description ##

Shinjiru is anime list management tool built with Qt designed to automatically update your progress to AniList and act an desktop manager for AniList.

## Installation ##

Shinjiru's [website](http://app.shinjiru.me/index.php) currently hosts Windows builds [here](http://app.shinjiru.me/latest.php).  
For other build needs feel free to [build](#Building) Shinjiru for yourself.


## Building ##

**Dependencies**

Shinjiru requires a few dependencies to compile and work properly, fortunately they are relatively easy to setup.

- **Qt**         - [http://www.qt.io/download-open-source/](http://www.qt.io/download-open-source/)
- **OpenSSL**    - [http://www.openssl.org/](http://www.openssl.org/)

The following dependencies should be cloned and place into the lib folder.

- **QtAwesome**  - [https://github.com/Kazakuri/QtAwesome](https://github.com/Kazakuri/QtAwesome)
- **AniListAPI** - [https://github.com/Kazakuri/AniListAPI](https://github.com/Kazakuri/AniListAPI)
- **Anitomy**    - [https://github.com/erengy/anitomy](https://github.com/erengy/anitomy)
- **Fervor**     - [https://github.com/pypt/fervor](https://github.com/pypt/fervor)

**app.h**

The file `app.h` contains two const QStrings, `api_id` and `api_secret`.  
Set the two aforementioned variables to your own client id and client secret if you are building it yourself, mine belong to me. :)

If you are using your own api information you should also change the update URL and authorization page URL.

## Contribute ##

If you want to contribute to Shinjiru feel free to create a pull request or an issue with suggestions/issues/features etc.

Make sure your code compiles if you're contributing.  
I don't have insanely high standards for code style but try to keep it within the [Google Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.html), or at the very least, 0 errors while running the file through cpplint.py.


## Contact ##

- **Email**: <admin@shinjiru.me>
- **Website**: [http://app.shinjiru.me](http://app.shinjiru.me)
- **GitHub**: [https://github.com/Kazakuri/Shinjiru](https://github.com/Kazakuri/Shinjiru)