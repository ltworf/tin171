all: server gui

server:
	cd server; rebar compile

gui:
	cd chinese-checkers/;./gen.sh;cd -

install_gui:
	python setup.py install --root=$(DESTDIR) --install-layout=deb --install-lib=/usr/share/games/chinese-checkers-client --install-scripts=/usr/games
	chmod a+x $(DESTDIR)/usr/share/games/chinese-checkers-client/chinese-checkers/chinese-checkers-gui.py
	chmod a+x $(DESTDIR)/usr/share/games/chinese-checkers-client/chinese-checkers/bot/bot.py

install_server:
	mkdir -p $(DESTDIR)/usr/share/games/chinese-checkers-server
	mkdir -p $(DESTDIR)/usr/games/
	cp server/ebin/* $(DESTDIR)/usr/share/games/chinese-checkers-server
	printf \#! >> $(DESTDIR)/usr/games/chinese-checkers-server
	echo /bin/sh >> $(DESTDIR)/usr/games/chinese-checkers-server
	echo INSTDIR=$(DESTDIR)/usr/share/chinese-checkers-server/ >> $(DESTDIR)/usr/games/chinese-checkers-server
	cat server/src/start >> $(DESTDIR)/usr/games/chinese-checkers-server
	chmod a+x $(DESTDIR)/usr/games/chinese-checkers-server

install: install_gui install_server

clean:
	rm -rf server/ebin
	rm -rf chinese-checkers/gui.py
	rm -rf build
