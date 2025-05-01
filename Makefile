CXX := clang++
CXX_LD := lld

BUILD_DIR := target

DEPS := gtkmm-3.0 glibmm-2.4 libcurl jsoncpp

setup:
	CXX=$(CXX) CXX_LD=$(CXX_LD) meson setup -Dbuildtype=debugoptimized $(BUILD_DIR)

setup-release:
	CXX=$(CXX) CXX_LD=$(CXX_LD) meson setup -Dbuildtype=release $(BUILD_DIR)

clean:
	CXX=$(CXX) CXX_LD=$(CXX_LD) meson setup --wipe $(BUILD_DIR)

compile:
	meson compile -C $(BUILD_DIR)

check:
	@for dep in $(DEPS); do \
		if pkg-config --exists $$dep; then \
			printf "%-15s \033[0;32m%s\033[0m\n" "$$dep" "✓ satisfied"; \
		else \
			printf "%-15s \033[0;31m%s\033[0m\n" "$$dep" "✗ not satisfied"; \
		fi; \
	done