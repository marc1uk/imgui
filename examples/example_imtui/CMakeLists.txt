#cmake_minimum_required (VERSION 3.10)
cmake_minimum_required (VERSION 3.7)
project(imtui)

set(CMAKE_EXPORT_COMPILE_COMMANDS "on")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
#    set(IMTUI_STANDALONE ON)
#    include(cmake/GitVars.cmake)
#else()
#    set(IMTUI_STANDALONE OFF)
#endif()
set(IMTUI_STANDALONE ON)

if (EMSCRIPTEN)
    set(IMTUI_SUPPORT_NCURSES_DEFAULT OFF)
    set(IMTUI_SUPPORT_CURL_DEFAULT OFF)
    set(IMTUI_LIBRARY_TYPE STATIC)
else()
    set(IMTUI_SUPPORT_NCURSES_DEFAULT ON)
    set(IMTUI_SUPPORT_CURL_DEFAULT ON)
    if (BUILD_SHARED_LIBS)
        set(IMTUI_LIBRARY_TYPE SHARED)
    else ()
        set(IMTUI_LIBRARY_TYPE STATIC)
    endif()
endif()

# options

option(IMTUI_ALL_WARNINGS            "imtui: enable all compiler warnings" ${IMTUI_STANDALONE})
option(IMTUI_ALL_WARNINGS_3RD_PARTY  "imtui: enable all compiler warnings in 3rd party libs" OFF)

option(IMTUI_SANITIZE_THREAD         "imtui: enable thread sanitizer" OFF)
option(IMTUI_SANITIZE_ADDRESS        "imtui: enable address sanitizer" OFF)
option(IMTUI_SANITIZE_UNDEFINED      "imtui: enable undefined sanitizer" OFF)

option(IMTUI_SUPPORT_NCURSES         "imtui: support for libncurses" ${IMTUI_SUPPORT_NCURSES_DEFAULT})
option(IMTUI_SUPPORT_CURL            "imtui: support for libcurl" ${IMTUI_SUPPORT_CURL_DEFAULT})

option(IMTUI_BUILD_EXAMPLES          "imtui: build examples" ${IMTUI_STANDALONE})

# sanitizers

if (IMTUI_SANITIZE_THREAD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

if (IMTUI_SANITIZE_ADDRESS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if (IMTUI_SANITIZE_UNDEFINED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

# dependencies

if (IMTUI_SUPPORT_NCURSES)
    find_package(Curses REQUIRED)
    CHECK_LIBRARY_EXISTS("${CURSES_NCURSES_LIBRARY}"
                         nodelay "" CURSES_NCURSES_HAS_NODELAY)
    if(NOT CURSES_NCURSES_HAS_NODELAY)
            find_library(CURSES_EXTRA_LIBRARY tinfo)
            CHECK_LIBRARY_EXISTS("${CURSES_EXTRA_LIBRARY}"
                                    nodelay "" CURSES_TINFO_HAS_NODELAY)
    endif()
    if(CURSES_EXTRA_LIBRARY)
            set(CURSES_LIBRARIES ${CURSES_LIBRARIES} ${CURSES_EXTRA_LIBRARY})
    endif()
endif()

# main

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif ()

#################################################
#add_subdirectory(third-party)
#################################################
# third-party/CMakeLists.txt
#################################################
if (IMTUI_ALL_WARNINGS_3RD_PARTY)
    if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    else()
        # todo : windows
    endif()
endif()

if (APPLE)
    set(ADDITIONAL_LIBRARIES "-framework Cocoa")
else (APPLE)
    unset(ADDITIONAL_LIBRARIES)
endif (APPLE)

#add_library(imgui ${IMTUI_LIBRARY_TYPE}
#    imgui/imgui/imgui.cpp
#    imgui/imgui/imgui_draw.cpp
#    imgui/imgui/imgui_demo.cpp
#    imgui/imgui/imgui_widgets.cpp
#    )
add_library(imgui ${IMTUI_LIBRARY_TYPE}
    ../../imgui.cpp
    ../../imgui_draw.cpp
    ../../imgui_demo.cpp
    ../../imgui_widgets.cpp
    )

target_include_directories(imgui INTERFACE
    ../
    )

target_include_directories(imgui PRIVATE
    ../
    )

target_link_libraries(imgui PRIVATE
    ${ADDITIONAL_LIBRARIES}
    )
#################################################

if (IMTUI_ALL_WARNINGS)
    if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    else()
        # todo : windows
    endif()
endif()

#################################################
#add_subdirectory(src)
##########################
# src/CMakeLists.txt
##########################
# core

add_library(imtui-core ${IMTUI_LIBRARY_TYPE}
    ../imtui-impl-text.cpp
    )

target_include_directories(imtui-core PUBLIC
    ./include
    )

target_link_libraries(imtui-core PUBLIC
    imgui
    )

target_link_libraries(imtui-core PRIVATE
    ${CMAKE_DL_LIBS}
    )

# ncurses

if (IMTUI_SUPPORT_NCURSES)
    add_library(imtui-ncurses ${IMTUI_LIBRARY_TYPE}
        ../imtui-impl-ncurses.cpp
        )

    target_include_directories(imtui-core PUBLIC
        ${CURSES_INCLUDE_DIR}
        )

    target_link_libraries(imtui-ncurses PUBLIC
        imtui-core
        ${CURSES_LIBRARIES}
        )
endif()

# emscripten

if (EMSCRIPTEN)
    add_library(imtui-emscripten ${IMTUI_LIBRARY_TYPE}
        ../imtui-impl-emscripten.cpp
        )

    target_link_libraries(imtui-emscripten PUBLIC
        imtui-core
        )
endif()

#################################################
#if (IMTUI_STANDALONE AND IMTUI_BUILD_EXAMPLES)
#    add_subdirectory(examples)
#endif()
#################################################
## examples/CMakeLists.txt
#################################################
find_package(Threads REQUIRED)

add_library(imtui-examples-common ${IMTUI_LIBRARY_TYPE}
#    imtui-demo.cpp
    )

target_link_libraries(imtui-examples-common PRIVATE
    imtui-core
    )

if (EMSCRIPTEN)
    add_subdirectory(emscripten0)
#    add_subdirectory(hnterm)
    return()
endif()

if (IMTUI_SUPPORT_NCURSES)
    add_subdirectory(ncurses0)

    if (IMTUI_SUPPORT_CURL)
        if (IMTUI_SUPPORT_CURL)
            find_package(CURL REQUIRED)
        endif()

#        add_subdirectory(hnterm)
    endif()
endif()
