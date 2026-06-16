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
    id: root
    color: "#111111"
    border.color: "#444444"

    Layout.fillWidth: true
    Layout.fillHeight: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Log"
                color: "white"
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }
        }

        ListView {
            id: logView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Backend.logLines

            delegate: Text {
                required property string modelData
                width: logView.width
                text: modelData
                color: "#d0d0d0"
                font.family: "monospace"
                wrapMode: Text.WrapAnywhere
            }

            Connections {
                target: Backend
                function onLogLinesChanged() {
                    logView.positionViewAtEnd()
                }
            }
        }
    }
}
