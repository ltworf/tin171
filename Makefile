all: server gui

server:
	cd server; rebar compile

gui:
	cd chinese-checkers/;./gen.sh;cd -

install_gui:
	python setup.py install --root=$(DESTDIR) --install-layout=deb --install-lib=/usr/share/chinese-checkers-client --install-scripts=/usr/games
	chmod a+x $(DESTDIR)/usr/share/chinese-checkers-client/chinese-checkers/chinese-checkers-gui.py
	chmod a+x $(DESTDIR)/usr/share/chinese-checkers-client/chinese-checkers/bot/bot.py

install_server:
	echo "TODO"