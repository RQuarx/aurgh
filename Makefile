CXX ?= clang++
CXX_LD ?= lld

TARGET_DIR  ?= ./target
GTK_VERSION ?= 4


all:
	@if [ ! -d "$(TARGET_DIR)" ]; then \
		CXX=$(CXX) CXX_LD=$(CXX_LD) meson setup $(TARGET_DIR); \
	fi

	CXX=$(CXX) CXX_LD=$(CXX_LD) meson configure $(TARGET_DIR) -Dgtk-version=$(GTK_VERSION)
	CXX=$(CXX) CXX_LD=$(CXX_LD) meson compile -C $(TARGET_DIR)


install:
	mkdir -p /usr/share/aurgh/
	cp -r ui /usr/share/aurgh/
	cp target/source/aurgh/aurgh /usr/bin/
	cp target/source/helper/helper /usr/share/aurgh/

	cp assets/aurgh.desktop /usr/share/applications/
	chmod a+x assets/run_aurgh.sh
	cp assets/run_aurgh.sh /usr/share/aurgh/

	mkdir /etc/aurgh
	cp config.json /etc/aurgh
	chmod -R a+rw /etc/aurgh

uninstall:
	rm -rf /usr/share/aurgh
	rm /usr/share/applications/aurgh.desktop
	rm /usr/bin/aurgh

clean:
	meson compile -C $(TARGET_DIR) --clean