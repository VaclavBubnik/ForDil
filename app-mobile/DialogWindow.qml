/*
    Copyright 2025 - Present MENDELU

    This file is part of ForDil.

    ForDil is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ForDil is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ForDil. If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import ForDil
import "." 1.0

Rectangle {
    id: dialogWindow

    anchors.fill: parent
    anchors.margins: 0
    color: "#ffffff"

    default property alias content: dialogContent.content
    property alias dialogHeader: dialogHeader
    property bool hasClose: false

    function resetView() {
        dialogContent.resetView()
    }

    onVisibleChanged: {
        if (visible) {
            dialogWindow.resetView()
        }
    }

    // Layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 1. Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: dialogHeader.implicitHeight

            Text {
                id: dialogHeader
                text: ""
                color: "#26787A"
                font.bold: true
                font.pixelSize: 18
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                anchors.rightMargin: dialogWindow.hasClose ? 50 : 0
                width: dialogWindow.hasClose ? (parent.width - 50) : parent.width
            }
        }

        // 2. Scrollable content area
        ZoomableView {
            id: dialogContent
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}

