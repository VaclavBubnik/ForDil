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

project(ForDilTest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui)

qt_standard_project_setup()

qt_add_executable(test
    main.cpp
    ../backend/c_chart.cpp
    ../backend/c_data.cpp
    ../backend/c_mappainter.cpp
    ../backend/c_pdf.cpp
    ../backend/c_result.cpp
    ../backend/c_type.cpp
    ../3rdparty/QXlsx/source/xlsxabstractooxmlfile.cpp
    ../3rdparty/QXlsx/source/xlsxabstractsheet.cpp
    ../3rdparty/QXlsx/source/xlsxcell.cpp
    ../3rdparty/QXlsx/source/xlsxcellformula.cpp
    ../3rdparty/QXlsx/source/xlsxcelllocation.cpp
    ../3rdparty/QXlsx/source/xlsxcellrange.cpp
    ../3rdparty/QXlsx/source/xlsxcellreference.cpp
    ../3rdparty/QXlsx/source/xlsxchart.cpp
    ../3rdparty/QXlsx/source/xlsxchartsheet.cpp
    ../3rdparty/QXlsx/source/xlsxcolor.cpp
    ../3rdparty/QXlsx/source/xlsxconditionalformatting.cpp
    ../3rdparty/QXlsx/source/xlsxcontenttypes.cpp
    ../3rdparty/QXlsx/source/xlsxdatavalidation.cpp
    ../3rdparty/QXlsx/source/xlsxdatetype.cpp
    ../3rdparty/QXlsx/source/xlsxdocpropsapp.cpp
    ../3rdparty/QXlsx/source/xlsxdocpropscore.cpp
    ../3rdparty/QXlsx/header/xlsxdocument.h
    ../3rdparty/QXlsx/source/xlsxdocument.cpp
    ../3rdparty/QXlsx/source/xlsxdrawing.cpp
    ../3rdparty/QXlsx/source/xlsxdrawinganchor.cpp
    ../3rdparty/QXlsx/source/xlsxformat.cpp
    ../3rdparty/QXlsx/source/xlsxmediafile.cpp
    ../3rdparty/QXlsx/source/xlsxnumformatparser.cpp
    ../3rdparty/QXlsx/source/xlsxrelationships.cpp
    ../3rdparty/QXlsx/source/xlsxrichstring.cpp
    ../3rdparty/QXlsx/source/xlsxsharedstrings.cpp
    ../3rdparty/QXlsx/source/xlsxsimpleooxmlfile.cpp
    ../3rdparty/QXlsx/source/xlsxstyles.cpp
    ../3rdparty/QXlsx/source/xlsxtheme.cpp
    ../3rdparty/QXlsx/source/xlsxutility.cpp
    ../3rdparty/QXlsx/source/xlsxworkbook.cpp
    ../3rdparty/QXlsx/source/xlsxworksheet.cpp
    ../3rdparty/QXlsx/source/xlsxzipreader.cpp
    ../3rdparty/QXlsx/source/xlsxzipwriter.cpp
)

target_include_directories(
    test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QXlsx/header
    ${CMAKE_CURRENT_SOURCE_DIR}/../backend
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test PRIVATE
    Qt6::Core
    Qt6::Gui
)
