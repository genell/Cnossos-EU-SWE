# Installation

## Dependencies (development)

* [Autoconf](https://www.gnu.org/software/autoconf/) for auto config/makefile generation
* [Libtool](https://www.gnu.org/software/libtool/) for shared library builds
* [GCC](https://www.gnu.org/software/gcc) build chain
* [Octave](https://www.gnu.org/software/octave/) (For Octave plugin)

## Reconfigure

```bash
> autoreconf --install
> ./configure
> make
> make checkdist
```

> If you get errors for missing .Po-files, run `make -k` (forces make to continue despite errors). After that, the dep files should have been created.
