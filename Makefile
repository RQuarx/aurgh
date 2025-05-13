install:
	mkdir -p /usr/share/aurgh/
	cp -r ui /usr/share/aurgh/
	cp target/aurgh /usr/bin/

uninstall:
	rm -rf /usr/share/aurgh
	rm /usr/bin/aurgh