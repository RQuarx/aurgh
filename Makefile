CXX    ?= clang++
CXX_LD ?= lld

TARGET_DIR  := ./target
GTK_VERSION ?= 4


all:
	@if [ ! -d "$(TARGET_DIR)" ]; then \
		CXX=$(CXX) CXX_LD=$(CXX_LD) meson setup $(TARGET_DIR); \
	fi

ifeq ($(GTK_VERSION),4)
	meson configure $(TARGET_DIR) -Duse-gtk4=true
else ifeq ($(GTK_VERSION),3)
	meson configure $(TARGET_DIR) -Duse-gtk4=false
endif
	meson compile -C $(TARGET_DIR)


install:
	mkdir -p /usr/share/aurgh/
	cp -r ui /usr/share/aurgh/
	cp target/source/aurgh/aurgh /usr/bin/
	cp target/source/helper/helper /usr/share/aurgh/

	cp assets/aurgh.desktop /usr/share/applications/
	chmod a+x assets/run_aurgh.sh
	cp assets/run_aurgh.sh /usr/share/aurgh/

uninstall:
	rm -rf /usr/share/aurgh
	rm /usr/share/applications/aurgh.desktop
	rm /usr/bin/aurgh