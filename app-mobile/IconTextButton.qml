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
import QtQuick.Controls
import QtQuick.Layouts
import ForDil

Button {
    id: control

    // Exposed API
    property alias iconSource: icon.source
    property int iconWidth: 25
    property int iconHeight: 25
    property color backgroundColor: "#41B28E"
    property color borderColor: "#26787A"
    property color textColor: "white"
    property string secondaryText: ""

    padding: 6

    contentItem: Item {
        anchors.fill: parent
        //anchors.margins: 5

        // icon
        Image {
            id: icon
            source: ""
            width: control.iconWidth
            height: control.iconHeight
            fillMode: Image.PreserveAspectFit
            visible: source !== ""
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
        }

        // single-line (centered)
        Text {
            id: primarySingle
            visible: control.secondaryText === ""
            text: control.text
            color: control.textColor
            font.pixelSize: 16
            anchors.centerIn: parent
        }

        // two-line (top/bottom, both centered horizontally)
        Column {
            id: twoLines
            visible: control.secondaryText !== ""
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin: 8
                rightMargin: 8
            }

            Text {
                text: control.text
                color: control.textColor
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                width: parent.width
            }

            Text {
                text: control.secondaryText
                color: control.textColor
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                width: parent.width
            }
        }
    }

    background: Rectangle {
        radius: 7
        color: control.backgroundColor
        border.width: 2
        border.color: control.borderColor
    }
}
