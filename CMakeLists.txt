cmake_minimum_required(VERSION 3.7)
project(sjasmplus)

SET(SJASMPLUS_VERSION 20190306.1+++WiP)

set(CMAKE_CXX_STANDARD 14)

configure_file(
        "${PROJECT_SOURCE_DIR}/sjasmplus_conf.h.in"
        "${PROJECT_BINARY_DIR}/sjasmplus_conf.h"
)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package (Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIR})

set(SOURCE_FILES
#        resources/bin2c/bin2c.cpp
#        resources/SaveTAP_ZX_Spectrum_128K.bin.h
#        resources/SaveTAP_ZX_Spectrum_256K.bin.h
#        resources/SaveTAP_ZX_Spectrum_48K.bin.h
        sjasm.cpp
        codeemitter.cpp
        codeemitter.h
        directives.cpp
        directives.h
        errors.cpp
        errors.h
        fs.h
        fsutil.cpp
        fsutil.h
        global.cpp
        global.h
        io_snapshots.cpp
        io_snapshots.h
        io_tape.cpp
        io_tape.h
        io_trd.cpp
        io_trd.h
        labels.cpp
        labels.h
        listing.cpp
        listing.h
        lua_lpack.c
        lua_lpack.h
        lua_sjasm.cpp
        lua_sjasm.h
        lua_support.cpp
        lua_support.h
        memory.cpp
        memory.h
        modules.cpp
        modules.h
        options.cpp
        options.h
        parser.cpp
        parser.h
        parser/define.h
        parser/define.cpp
        parser/macro.h
        parser/macro.cpp
        parser/struct.h
        parser/struct.cpp
        reader.cpp
        reader.h
        sjio.cpp
        sjio.h
        support.cpp
        support.h
        tables.cpp
        tables.h
        util.cpp
        util.h
        z80.cpp
        z80.h
        zxspectrum.cpp
        zxspectrum.h
        )
list(TRANSFORM SOURCE_FILES PREPEND "src/" OUTPUT_VARIABLE SOURCE_FILES)

set(LUA_DIR "3rdparty/lua5.1/")
set(LUA_FILES
        lapi.c
        lapi.h
        lauxlib.c
        lauxlib.h
        lbaselib.c
        lcode.c
        lcode.h
        ldblib.c
        ldebug.c
        ldebug.h
        ldo.c
        ldo.h
        ldump.c
        lfunc.c
        lfunc.h
        lgc.c
        lgc.h
        linit.c
        liolib.c
        llex.c
        llex.h
        llimits.h
        lmathlib.c
        lmem.c
        lmem.h
        loadlib.c
        lobject.c
        lobject.h
        lopcodes.c
        lopcodes.h
        loslib.c
        lparser.c
        lparser.h
        lstate.c
        lstate.h
        lstring.c
        lstring.h
        lstrlib.c
        ltable.c
        ltable.h
        ltablib.c
        ltm.c
        ltm.h
        lua.h
        lua.hpp
        luaconf.h
        lualib.h
        lundump.c
        lundump.h
        lvm.c
        lvm.h
        lzio.c
        lzio.h
        )
list(TRANSFORM LUA_FILES PREPEND ${LUA_DIR} OUTPUT_VARIABLE LUA_FILES)

set(TOLUAPP_DIR "3rdparty/tolua++/")
set(TOLUAPP_FILES
        tolua++.h
        tolua_event.c
        tolua_event.h
        tolua_is.c
        tolua_map.c
        tolua_push.c
        tolua_to.c
        )
list(TRANSFORM TOLUAPP_FILES PREPEND ${TOLUAPP_DIR} OUTPUT_VARIABLE TOLUAPP_FILES)

list(APPEND SOURCE_FILES ${LUA_FILES} ${TOLUAPP_FILES})

set(PEGTL_DIR "3rdparty/PEGTL/")

if (WIN32)
  if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set (CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> -O coff <FLAGS> <DEFINES> <SOURCE> <OBJECT>")
  endif ()
  set (SOURCE_FILES ${SOURCE_FILES} "${PROJECT_SOURCE_DIR}/win32/sjasmplus.rc")
endif ()

include_directories(BEFORE sjasm ${LUA_DIR} ${TOLUAPP_DIR} "${PEGTL_DIR}include")

add_executable(sjasmplus ${SOURCE_FILES})

target_link_libraries (sjasmplus ${Boost_LIBRARIES})

add_subdirectory(tests tests EXCLUDE_FROM_ALL)
