# Copyright 2025 - Present MENDELU
#
# This file is part of ForDil.
#
# ForDil is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ForDil is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ForDil. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)

project(ForDil VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Qml Positioning Sensors Quick)

qt_policy(SET QTP0002 NEW)

qt_standard_project_setup(REQUIRES 6.5)

qt6_add_resources(RESOURCE_FILES resources.qrc)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
add_compile_definitions(APP_BUILD_DATE="${BUILD_DATE}")

qt_add_executable(appForDil
    c_main.cpp
    ${RESOURCE_FILES}
)

target_compile_definitions(appForDil PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
    $<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>
    $<$<CONFIG:Release>:QT_NO_WARNING_OUTPUT>
)

set_target_properties(appForDil PROPERTIES
    QT_ANDROID_MIN_SDK_VERSION 33
    QT_ANDROID_TARGET_SDK_VERSION 36
    QT_ANDROID_COMPILE_SDK_VERSION 36
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)

qt_add_qml_module(appForDil
    URI ForDil
    VERSION 1.0
    QML_FILES ChapterHeader.qml
    QML_FILES CloseButton.qml
    QML_FILES Components.qml
    QML_FILES CustomPopup.qml
    QML_FILES DialogWindow.qml
    QML_FILES Globals.qml
    QML_FILES IconTextButton.qml
    QML_FILES LogWindow.qml
    QML_FILES MapView.qml
    QML_FILES SiteSelection.qml
    QML_FILES TextButton.qml
    QML_FILES ZoomableView.qml
    QML_FILES PageAbout.qml
    QML_FILES PageAdvancedResults.qml
    QML_FILES PageDiscoveryPart.qml
    QML_FILES PageGlobalResults.qml
    QML_FILES PageHome.qml
    QML_FILES PageHotspots.qml
    QML_FILES PageMain.qml
    QML_FILES PageMarteloscope.qml
    QML_FILES PageSettings.qml
    QML_FILES PageSiteDiscovery.qml
    QML_FILES PageSiteMarteloscope.qml
    QML_FILES PageSiteTravailloscope.qml
    QML_FILES PageTravailloscope.qml
    QML_FILES PageValuation.qml
    RESOURCES qmldir
    SOURCES c_backend.h c_backend.cpp
    SOURCES ../backend/c_chart.h ../backend/c_chart.cpp
    SOURCES ../backend/c_data.h ../backend/c_data.cpp
    SOURCES ../backend/c_mappainter.h ../backend/c_mappainter.cpp
    SOURCES ../backend/c_pdf.h ../backend/c_pdf.cpp
    SOURCES ../backend/c_result.h ../backend/c_result.cpp
    SOURCES ../backend/c_type.h ../backend/c_type.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxabstractooxmlfile_p.h ../3rdparty/QXlsx/source/xlsxabstractooxmlfile.cpp ../3rdparty/QXlsx/header/xlsxabstractooxmlfile.h
    SOURCES ../3rdparty/QXlsx/header/xlsxabstractsheet_p.h ../3rdparty/QXlsx/source/xlsxabstractsheet.cpp ../3rdparty/QXlsx/header/xlsxabstractsheet.h
    SOURCES ../3rdparty/QXlsx/header/xlsxcell_p.h ../3rdparty/QXlsx/source/xlsxcell.cpp ../3rdparty/QXlsx/header/xlsxcell.h
    SOURCES ../3rdparty/QXlsx/header/xlsxcellformula_p.h ../3rdparty/QXlsx/source/xlsxcellformula.cpp ../3rdparty/QXlsx/header/xlsxcellformula.h
    SOURCES ../3rdparty/QXlsx/source/xlsxcelllocation.cpp ../3rdparty/QXlsx/header/xlsxcelllocation.h
    SOURCES ../3rdparty/QXlsx/source/xlsxcellrange.cpp ../3rdparty/QXlsx/header/xlsxcellrange.h
    SOURCES ../3rdparty/QXlsx/source/xlsxcellreference.cpp ../3rdparty/QXlsx/header/xlsxcellreference.h
    SOURCES ../3rdparty/QXlsx/header/xlsxchart_p.h ../3rdparty/QXlsx/source/xlsxchart.cpp ../3rdparty/QXlsx/header/xlsxchart.h
    SOURCES ../3rdparty/QXlsx/header/xlsxchartsheet_p.h ../3rdparty/QXlsx/source/xlsxchartsheet.cpp ../3rdparty/QXlsx/header/xlsxchartsheet.h
    SOURCES ../3rdparty/QXlsx/header/xlsxcolor_p.h ../3rdparty/QXlsx/source/xlsxcolor.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxconditionalformatting_p.h ../3rdparty/QXlsx/source/xlsxconditionalformatting.cpp ../3rdparty/QXlsx/header/xlsxconditionalformatting.h
    SOURCES ../3rdparty/QXlsx/header/xlsxcontenttypes_p.h ../3rdparty/QXlsx/source/xlsxcontenttypes.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxdatavalidation_p.h ../3rdparty/QXlsx/source/xlsxdatavalidation.cpp ../3rdparty/QXlsx/header/xlsxdatavalidation.h
    SOURCES ../3rdparty/QXlsx/source/xlsxdatetype.cpp ../3rdparty/QXlsx/header/xlsxdatetype.h
    SOURCES ../3rdparty/QXlsx/header/xlsxdocpropsapp_p.h ../3rdparty/QXlsx/source/xlsxdocpropsapp.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxdocpropscore_p.h ../3rdparty/QXlsx/source/xlsxdocpropscore.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxdocument_p.h ../3rdparty/QXlsx/source/xlsxdocument.cpp ../3rdparty/QXlsx/header/xlsxdocument.h
    SOURCES ../3rdparty/QXlsx/header/xlsxdrawing_p.h ../3rdparty/QXlsx/source/xlsxdrawing.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxdrawinganchor_p.h ../3rdparty/QXlsx/source/xlsxdrawinganchor.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxformat_p.h ../3rdparty/QXlsx/source/xlsxformat.cpp ../3rdparty/QXlsx/header/xlsxformat.h
    SOURCES ../3rdparty/QXlsx/header/xlsxglobal.h
    SOURCES ../3rdparty/QXlsx/header/xlsxmediafile_p.h ../3rdparty/QXlsx/source/xlsxmediafile.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxnumformatparser_p.h ../3rdparty/QXlsx/source/xlsxnumformatparser.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxrelationships_p.h ../3rdparty/QXlsx/source/xlsxrelationships.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxrichstring_p.h ../3rdparty/QXlsx/source/xlsxrichstring.cpp ../3rdparty/QXlsx/header/xlsxrichstring.h
    SOURCES ../3rdparty/QXlsx/header/xlsxsharedstrings_p.h ../3rdparty/QXlsx/source/xlsxsharedstrings.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxsimpleooxmlfile_p.h ../3rdparty/QXlsx/source/xlsxsimpleooxmlfile.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxstyles_p.h ../3rdparty/QXlsx/source/xlsxstyles.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxtheme_p.h ../3rdparty/QXlsx/source/xlsxtheme.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxutility_p.h ../3rdparty/QXlsx/source/xlsxutility.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxworkbook_p.h ../3rdparty/QXlsx/source/xlsxworkbook.cpp ../3rdparty/QXlsx/header/xlsxworkbook.h
    SOURCES ../3rdparty/QXlsx/header/xlsxworksheet_p.h ../3rdparty/QXlsx/source/xlsxworksheet.cpp ../3rdparty/QXlsx/header/xlsxworksheet.h
    SOURCES ../3rdparty/QXlsx/header/xlsxzipreader_p.h ../3rdparty/QXlsx/source/xlsxzipreader.cpp
    SOURCES ../3rdparty/QXlsx/header/xlsxzipwriter_p.h ../3rdparty/QXlsx/source/xlsxzipwriter.cpp
)

target_include_directories(
    appForDil
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QXlsx/header
    ${CMAKE_CURRENT_SOURCE_DIR}/../backend
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appForDil PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.mendelu.fordil
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appForDil
    PRIVATE Qt6::Core
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Qml
    PRIVATE Qt6::Positioning
    PRIVATE Qt6::Sensors
    PRIVATE Qt6::Quick
)

set_property(TARGET appForDil PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)

include(GNUInstallDirs)
install(TARGETS appForDil
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
