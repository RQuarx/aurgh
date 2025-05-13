install:
	mkdir -p /usr/share/aurgh/
	cp -r ui /usr/share/aurgh/
	cp target/aurgh /usr/bin/

	cp assets/aurgh.desktop /usr/share/applications/
	chmod a+x assets/run_aurgh.sh
	cp assets/run_aurgh.sh /usr/share/aurgh/

uninstall:
	rm -rf /usr/share/aurgh
	rm /usr/share/applications/aurgh.desktop
	rm /usr/bin/aurgh