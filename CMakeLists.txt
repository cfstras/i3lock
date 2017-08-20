cmake_minimum_required(VERSION 3.7)
project(i3lock)
find_package(PkgConfig)

#set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-pipe -Wall -std=c99 -O2 -g")
set(CMAKE_CXX_FLAGS "-D_GNU_SOURCE")
add_definitions("-D_GNU_SOURCE")

pkg_check_modules(XLIBS REQUIRED cairo xcb-xinerama xcb-atom xcb-image xcb-xkb xkbcommon xkbcommon-x11)
include_directories(${XLIBS_INCLUDE_DIRS})
link_directories(${XLIBS_LIBRARY_DIRS})
add_definitions(${XLIBS_CFLAGS_OTHER})

execute_process(COMMAND git describe --tags --abbrev=0
        OUTPUT_VARIABLE VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND git describe --tags --always
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git log --pretty=format:%cd --date=short -n1
        OUTPUT_VARIABLE GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

set(I3LOCK_VERSION "${GIT_TAG} ${GIT_DATE}")
add_definitions("-DVERSION=\"${I3LOCK_VERSION}\"")

set(SOURCE_FILES
        cursors.h
        i3lock.c
        i3lock.h
        unlock_indicator.c
        unlock_indicator.h
        xcb.c
        xcb.h
        xinerama.c
        xinerama.h
        blur.c
        blur.h)

add_executable(i3lock ${SOURCE_FILES})

set(LIBS ${LIBS} ${XLIBS_LIBRARIES})

target_link_libraries(i3lock ev m ${LIBS})
target_link_libraries(i3lock pam)
