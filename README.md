Chinese Checkers
================

The original project started for a university course about Artificial
Intelligence. In this forked repository the aim is to make it usable by a more
general public.

Requirements
============
For the server:
- rebar
- erlang-base (or whatever package providing /usr/bin/erl)

For the client:
- Python 2.7 (perhaps it works with 2.6 but it's not tested).
- python-yappy
- python-qt4
- pyqt4-dev-tools (to compile the .py from .ui, it's not a runtime need)
- python-notify2 (not strictly necessary but it adds a nice feature)

Install
=======
The preferred way of installing it is to run
```
dpkg-buildpackage
```
and then install the .deb files, since the debian/ directory is provided.

alternatively, run
```
make
make install
```
make install supports DESTDIR, 
```
make DESTDIR=/tmp/test/ install
```

Usage
=====
The game has a distributed architecture. Bots and GUIs are clients, so they
need to connect to a server.
The server is able to handle several clients and matches at the same time.

The various AI are optimized for different scenarios, so they might perform
well against a single opponent and very poorly against multiple opponents, or
viceversa.
