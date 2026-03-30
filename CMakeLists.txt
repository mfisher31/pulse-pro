cmake_minimum_required(VERSION 3.25)

project(pulse-pro VERSION 0.1.0 LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(cmake/SetupQt.cmake)

set(PULSE_SOURCES
    src/main.cpp
    src/screencapturepreview.cpp
    src/screencapturepreview.hpp
    src/screenlistmodel.hpp
    src/screenlistmodel.cpp
    src/windowlistmodel.hpp
    src/windowlistmodel.cpp
    src/regionselectionoverlay.hpp
    src/regionselectionoverlay.cpp
)

if(APPLE)
    list(APPEND PULSE_SOURCES
        src/macoswindowutils.h
        src/macoswindowutils.mm
    )
endif()

qt_add_executable(pulse-pro
    MANUAL_FINALIZATION
    ${PULSE_SOURCES}
)

target_link_libraries(pulse-pro PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets)

set_target_properties(pulse-pro PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)

if(NOT LINUX)
    set_target_properties(pulse-pro PROPERTIES OUTPUT_NAME "PulsePro")
endif()

if(IOS)
    qt_add_ios_ffmpeg_libraries(pulse-pro)
endif()

set(PULSE_INSTALLDIR ".")
install(TARGETS pulse-pro
    RUNTIME DESTINATION "${PULSE_INSTALLDIR}"
    BUNDLE DESTINATION "${PULSE_INSTALLDIR}"
    LIBRARY DESTINATION "${PULSE_INSTALLDIR}"
)

qt_generate_deploy_app_script(
    TARGET pulse-pro
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

qt_finalize_executable(pulse-pro)
