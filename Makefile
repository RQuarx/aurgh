CXX := clang++
CXX_LD := lld

MESON := CXX=$(CXX) CXX_LD=$(CXX_LD) $(shell which meson)

BUILD_DIR := target

DEPS := gtkmm-3.0 glibmm-2.4 libcurl jsoncpp

all:
	@echo "Available options are: setup, setup-release, clean, compile, check."

setup:
	$(MESON) setup -Dbuildtype=debugoptimized $(BUILD_DIR)

setup-release:
	$(MESON) setup -Dbuildtype=release $(BUILD_DIR)

clean:
	$(MESON) setup --wipe $(BUILD_DIR)

compile:
	$(MESON) compile -C $(BUILD_DIR)

install-deps:
	@pacman -Syyu --noconfirm --noprogressbar --needed - < required.txt

check:
	@for dep in $(DEPS); do \
		if pkg-config --exists $$dep; then \
			printf "%-15s \033[0;32m%s\033[0m\n" "$$dep" "✓ satisfied"; \
		else \
			printf "%-15s \033[0;31m%s\033[0m\n" "$$dep" "✗ not satisfied"; \
		fi; \
	done