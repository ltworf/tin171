Chinese Checkers
================

The original project started for a university course about Artificial
Intelligence. In this forked repository the aim is to make it usable by a more
general public.

Install
=======
The preferred way of installing it is to run
```
dpkg-buildpackage
```
and then install the .deb files, since the debian/ directory is provided.

alternatively, use the setup.py and the Makefile to do a manual install,
and check the content of debian/rules to see the various steps to perform.

Usage
=====
The game has a distributed architecture. Bots and GUIs are clients, so they
need to connect to a server.
The server is able to handle several clients and matches at the same time.

The various AI are optimized for different scenarios, so they might perform
well against a single opponent and very poorly against multiple opponents, or
viceversa.
