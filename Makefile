PKG_CONFIG?=pkg-config

PKGS="wlroots-0.21" wayland-server xkbcommon
CFLAGS_PKG_CONFIG := $(shell $(PKG_CONFIG) --cflags $(PKGS))
LIBS := $(shell $(PKG_CONFIG) --libs $(PKGS))

CXXFLAGS := -g -Werror -DWLR_USE_UNSTABLE $(CFLAGS_PKG_CONFIG)
CXX := g++

all: fwm

fwm.o: fwm.cpp
	$(CXX) -c $< $(CXXFLAGS) -I. -o $@

fwm: fwm.o
	$(CXX) $^ $(CXXFLAGS) $(LIBS) -o $@

clean:
	rm -f fwm fwm.o

.PHONY: all clean