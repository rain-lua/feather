PKG_CONFIG?=pkg-config

PKGS="wlroots-0.21" wayland-server xkbcommon
CFLAGS_PKG_CONFIG := $(shell $(PKG_CONFIG) --cflags $(PKGS))
LIBS := $(shell $(PKG_CONFIG) --libs $(PKGS))

CXX := g++
CXXFLAGS := -g -Werror -DWLR_USE_UNSTABLE $(CFLAGS_PKG_CONFIG) -Isrc/include -Isrc/debug

SRC := $(shell find src -name '*.cpp')
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))

all: build feather

build:
	mkdir -p build

build/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

feather: $(OBJ)
	$(CXX) $^ $(CXXFLAGS) $(LIBS) -o build/feather

clean:
	rm -rf build

.PHONY: all clean build