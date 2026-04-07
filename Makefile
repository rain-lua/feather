PKG_CONFIG ?= pkg-config

PKGS := wlroots-0.21 wayland-server xkbcommon

MISSING_PKGS := $(strip \
  $(foreach p,$(PKGS), \
    $(if $(shell $(PKG_CONFIG) --exists $(p) 2>/dev/null || echo missing),$(p)) \
  ) \
)

ifneq ($(MISSING_PKGS),)
  $(error Missing required packages: $(MISSING_PKGS))
endif

CFLAGS_PKG := $(shell $(PKG_CONFIG) --cflags $(PKGS))
LIBS_PKG   := $(shell $(PKG_CONFIG) --libs $(PKGS))

LUA_VERSIONS := lua lua5.4 lua5.3 lua5.2 lua5.1 luajit

LUA_PKG := $(firstword \
  $(foreach v,$(LUA_VERSIONS), \
    $(if $(shell $(PKG_CONFIG) --exists $(v) 2>/dev/null && echo $(v)),$(v)) \
  ) \
)

ifeq ($(LUA_PKG),)
  $(error No Lua installation found. Tried: $(LUA_VERSIONS))
endif

CFLAGS_LUA := $(shell $(PKG_CONFIG) --cflags $(LUA_PKG))
LIBS_LUA   := $(shell $(PKG_CONFIG) --libs $(LUA_PKG))

CXX := g++
CXXFLAGS := -g -Werror -DWLR_USE_UNSTABLE \
  $(CFLAGS_PKG) $(CFLAGS_LUA) \
  -Isrc/include -Isrc/debug

SRC := $(shell find src -name '*.cpp')
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET := build/feather

define msg_color
	@echo -e "\033[34m[$(shell date +'%H:%M:%S')]\033[0m \033[$(1)m$(2)\033[0m"
endef

all: build $(TARGET)
	$(call msg_color,32,Build finished successfully)

build:
	$(call msg_color,33,Creating build directory...)
	mkdir -p build

build/%.o: src/%.cpp
	$(call msg_color,33,Compiling $< ...)
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(TARGET): $(OBJ)
	$(call msg_color,33,Linking $(TARGET) ...)
	$(CXX) $(OBJ) $(LIBS_PKG) $(LIBS_LUA) $(CXXFLAGS) -o $(TARGET)
	$(call msg_color,32,Target $(TARGET) built successfully)

clean:
	$(call msg_color,31,Cleaning build directory...)
	rm -rf build

.PHONY: all build clean