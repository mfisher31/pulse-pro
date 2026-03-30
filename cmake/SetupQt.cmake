# Copyright (C) 2026 Medical Informatics Engineering.
# SPDX-License-Identifier: LGPL-3.0-or-later

# Auto-discover Qt from the standard Qt Community installer location (~Qt/<version>/macos)
# so developers don't need to set CMAKE_PREFIX_PATH or any environment variables.
# Explicit CMAKE_PREFIX_PATH / Qt6_DIR still take precedence if set.
if((APPLE OR LINUX) AND NOT Qt6_DIR)
    if(APPLE)
        set(_qt_platform_dir "macos")
    else()
        set(_qt_platform_dir "gcc_64")
    endif()
    file(GLOB _qt_candidate_paths "$ENV{HOME}/Qt/*/${_qt_platform_dir}")
    if(_qt_candidate_paths)
        list(SORT _qt_candidate_paths ORDER DESCENDING)   # highest version first
        list(GET _qt_candidate_paths 0 _qt_root)
        list(PREPEND CMAKE_PREFIX_PATH "${_qt_root}")
        message(STATUS "pulse-pro: auto-detected Qt at ${_qt_root}")
    endif()
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} 6.5 REQUIRED COMPONENTS MultimediaWidgets Widgets)

if(QT_VERSION VERSION_LESS "6.5")
    message(FATAL_ERROR "Qt 6.5 or higher is required (found ${QT_VERSION}). "
        "QWindowCapture and QScreenCapture were introduced in Qt 6.5.")
endif()
