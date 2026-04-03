# Copyright (C) 2026 Medical Informatics Engineering.
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(pulse-pro-tests
    main.cpp
    screenlistmodel_test.hpp
    screenlistmodel_test.cpp
    windowlistmodel_test.hpp
    windowlistmodel_test.cpp
    regionselectionoverlay_test.hpp
    regionselectionoverlay_test.cpp
    ${CMAKE_SOURCE_DIR}/src/screenlistmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/windowlistmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/regionselectionoverlay.cpp
)

if(APPLE)
    target_sources(pulse-pro-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/windowutil.mm)
else()
    target_sources(pulse-pro-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/windowutil.cpp)
endif()

target_include_directories(pulse-pro-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(pulse-pro-tests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Test
)

add_test(NAME PulseProTests COMMAND pulse-pro-tests)
