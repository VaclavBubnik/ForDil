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

Popup {
    id: control

    property string title: ""
    required property string text
    property string text2: ""
    default property alias buttons: buttonsRow.data

    property color backgroundColor: "#D5EFE0"
    property color borderColor: "#41B28E"
    property color textColor: "black"
    property int fontSize: 16

    width: 350
    padding: 10

    modal: true
    focus: true

    parent: Overlay.overlay
    anchors.centerIn: parent

    contentItem: Column {
        width: control.availableWidth
        spacing: 10

        Text {
            visible: control.title.length > 0
            width: parent.width
            text: control.title
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            visible: control.title.length > 0
            width: parent.width
            height: 1
            color: "#26787A"
        }

        Text {
            width: parent.width
            text: control.text;
            font.pixelSize: control.fontSize
            color: control.textColor
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            visible: control.text2.length > 0
            width: parent.width
            text: control.text2;
            font.pixelSize: control.fontSize
            color: control.textColor
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Row {
            id: buttonsRow
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20
            visible: children.length > 0
        }
    }

    background: Rectangle {
        radius: 7
        color: control.backgroundColor
        border.width: 2
        border.color: control.borderColor
    }
}
