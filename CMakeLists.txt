cmake_minimum_required(VERSION 3.20)

project(feather LANGUAGES CXX)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET wlroots-0.19)
pkg_check_modules(WAYLAND_SERVER REQUIRED IMPORTED_TARGET wayland-server)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.5)
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)

pkg_get_variable(WAYLAND_PROTOCOLS_PKGDATADIR
    wayland-protocols
    pkgdatadir
)

find_program(WAYLAND_SCANNER
    NAMES wayland-scanner
    REQUIRED
)

set(PROTOCOL_DIR "${CMAKE_CURRENT_BINARY_DIR}/protocols")

set(XDG_XML
    "${WAYLAND_PROTOCOLS_PKGDATADIR}/stable/xdg-shell/xdg-shell.xml"
)

set(XDG_HEADER
    "${PROTOCOL_DIR}/xdg-shell-protocol.h"
)

set(XDG_CODE
    "${PROTOCOL_DIR}/xdg-shell-protocol.c"
)

add_custom_command(
    OUTPUT
        "${XDG_HEADER}"
        "${XDG_CODE}"

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${PROTOCOL_DIR}"

    COMMAND "${WAYLAND_SCANNER}"
        server-header
        "${XDG_XML}"
        "${XDG_HEADER}"

    COMMAND "${WAYLAND_SCANNER}"
        private-code
        "${XDG_XML}"
        "${XDG_CODE}"

    DEPENDS
        "${XDG_XML}"

    COMMENT "Generating xdg-shell protocol"
    VERBATIM
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(feather
    ${SOURCES}
    "${XDG_CODE}"
    "${XDG_HEADER}"
)

target_compile_definitions(feather PRIVATE
    WLR_USE_UNSTABLE
)

if(ENABLE_ASAN)
    target_compile_options(feather PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(feather PRIVATE
        -fsanitize=address
    )
endif()

if(ENABLE_UBSAN)
    target_compile_options(feather PRIVATE
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )

    target_link_options(feather PRIVATE
        -fsanitize=undefined
    )
endif()

target_compile_options(feather PRIVATE
    -Werror
)

target_include_directories(feather PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/debug"
    "${PROTOCOL_DIR}"
)

target_link_libraries(feather PRIVATE
    PkgConfig::WLROOTS
    PkgConfig::WAYLAND_SERVER
    PkgConfig::XKBCOMMON
    PkgConfig::LUA
)