
# Auto-discover Qt from the standard Qt Community installer location (~Qt/<version>/macos)
# so developers don't need to set CMAKE_PREFIX_PATH or any environment variables.
# Explicit CMAKE_PREFIX_PATH / Qt6_DIR still take precedence if set.
if(APPLE AND NOT Qt6_DIR)
    file(GLOB _qt_candidate_paths "$ENV{HOME}/Qt/*/macos")
    if(_qt_candidate_paths)
        list(SORT _qt_candidate_paths ORDER DESCENDING)   # highest version first
        list(GET _qt_candidate_paths 0 _qt_root)
        list(PREPEND CMAKE_PREFIX_PATH "${_qt_root}")
        message(STATUS "pulse-pro: auto-detected Qt at ${_qt_root}")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS MultimediaWidgets Widgets)
