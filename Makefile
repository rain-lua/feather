.PHONY: default all prebuild build clean

default: prebuild all

prebuild:
	@if [ -d build ]; then \
		$(MAKE) clean; \
	fi

PKG_CONFIG ?= pkg-config

PKGS := wlroots-0.19 wayland-server xkbcommon

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

LUA_PKG := lua5.5

CFLAGS_LUA := $(shell $(PKG_CONFIG) --cflags $(LUA_PKG))
LIBS_LUA   := $(shell $(PKG_CONFIG) --libs $(LUA_PKG))

WAYLAND_PROTOCOLS_DIR := $(shell $(PKG_CONFIG) --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER := wayland-scanner

BUILD_PROTO_DIR := build/protocols

XDG_XML := $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

XDG_HEADER := $(BUILD_PROTO_DIR)/xdg-shell-protocol.h
XDG_CODE   := $(BUILD_PROTO_DIR)/xdg-shell-protocol.c

PROTO_FILES := $(XDG_HEADER) $(XDG_CODE)

$(PROTO_FILES): $(XDG_XML)
	$(call msg_color,32,Generating xdg-shell...)
	@mkdir -p $(BUILD_PROTO_DIR)
	$(WAYLAND_SCANNER) client-header $< $(XDG_HEADER)
	$(WAYLAND_SCANNER) private-code $< $(XDG_CODE)

CXXFLAGS := -g -Werror -DWLR_USE_UNSTABLE \
  $(CFLAGS_PKG) $(CFLAGS_LUA) \
  -Isrc/include -Isrc/debug \
  -I$(BUILD_PROTO_DIR)

SRC := $(shell find src -name '*.cpp')
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET := build/feather

define msg_color
	@echo -e "\033[34m[$(shell date +'%H:%M:%S')]\033[0m \033[$(1)m$(2)\033[0m"
endef

all: build $(TARGET)
	$(call msg_color,32,Build finished successfully)

build:
	@mkdir -p build

build/%.o: src/%.cpp $(PROTO_FILES)
	$(call msg_color,33,Compiling $< ...)
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(TARGET): $(OBJ)
	$(call msg_color,33,Linking $(TARGET) ...)
	$(CXX) $(OBJ) $(LIBS_PKG) $(LIBS_LUA) $(CXXFLAGS) -o $(TARGET)
	$(call msg_color,32,Target $(TARGET) built successfully)

clean:
	$(call msg_color,31,Cleaning build directory...)
	rm -rf build

.PHONY: all build clean