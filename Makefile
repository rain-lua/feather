PKG_CONFIG ?= pkg-config

PKGS := wlroots-0.21 wayland-server xkbcommon
CFLAGS_PKG := $(shell $(PKG_CONFIG) --cflags $(PKGS))
LIBS_PKG   := $(shell $(PKG_CONFIG) --libs $(PKGS))

LUA_VERSIONS := lua lua5.4 lua5.3 lua5.2 lua5.1 luajit
CFLAGS_LUA :=
LIBS_LUA :=

define detect_lua
  $(foreach v,$(LUA_VERSIONS), \
    $(if $(shell $(PKG_CONFIG) --exists $(v) 2>/dev/null && echo yes), \
      $(eval CFLAGS_LUA := $(shell $(PKG_CONFIG) --cflags $(v))) \
      $(eval LIBS_LUA := $(shell $(PKG_CONFIG) --libs $(v))) \
      $(eval LUA_FOUND := yes) \
      $(break)) )
endef

$(eval $(detect_lua))

ifeq ($(LUA_FOUND),)
  $(error "No Lua installation found. Install Lua development files.")
endif


CXX := g++
CXXFLAGS := -g -Werror -DWLR_USE_UNSTABLE $(CFLAGS_PKG) $(CFLAGS_LUA) -Isrc/include -Isrc/debug

SRC := $(shell find src -name '*.cpp')
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET := build/feather

all: build $(TARGET)

build:
	mkdir -p build

build/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LIBS_PKG) $(LIBS_LUA) $(CXXFLAGS) -o $(TARGET)

clean:
	rm -rf build

.PHONY: all build clean