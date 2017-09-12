tclmixer
=====

[TclMixer](http://sqlitestudio.pl/tclmixer/) provides SDL_mixer (SDL) bindings for Tcl.
It allows to play multiple sounds simultaneously using a built-in software mixer.


License
=====

LGPL (ths same with TclMixer)


UNIX BUILD
=====

I just update configure and link to SDL2 and SDL2_mixer.
Only test on openSUSE. Users need install SDL2/SDL2_mixer development files first.

    sudo zypper install libSDL2-devel libSDL2_mixer-devel

Building under most UNIX systems is easy, just run the configure script
and then run make. For more information about the build process, see the
tcl/unix/README file in the Tcl src dist. The following minimal example
will install the extension in the /opt/tcl directory.

    $ cd tclmixer
    $ ./configure --prefix=/opt/tcl
    $ make
    $ make install

If you need setup directory containing tcl configuration (tclConfig.sh),
below is an example:

    $ cd tclmixer
    $ ./configure --with-tcl=/opt/activetcl/lib
    $ make
    $ make install


WINDOWS BUILD
=====

The recommended method to build extensions under windows is to use the
Msys + Mingw build process. This provides a Unix-style build while generating
native Windows binaries. Using the Msys + Mingw build tools means that you can
use the same configure script as per the Unix build to create a Makefile.

[SDL](https://www.libsdl.org/) and [SDL_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/)
provides runtime binaries for Windows platform and development libraries
for MinGW 32/64-bit.

You can use Msys/Mingw (32-bit) or MSYS2/MinGW-w64 (64-bit) to build
this extension after install SDL/SDL_mixer development libraries.

