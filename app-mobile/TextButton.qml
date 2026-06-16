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

    width: 150
    height: 40

    // Exposed API
    property color backgroundColor: "#41B28E"
    property color borderColor: "#26787A"
    property color textColor: "white"

    property color disabledBackgroundColor: "#D7EAE4"
    property color disabledBorderColor: "#A8C7BF"
    property color disabledTextColor: "#6B6B6B"

    padding: 6

    contentItem: Item {
        anchors.fill: parent

        Text {
            id: primarySingle
            text: control.text
            color: control.enabled ? control.textColor : control.disabledTextColor
            font.pixelSize: 16
            anchors.centerIn: parent
        }
    }

    background: Rectangle {
        radius: 7
        color: control.enabled ? control.backgroundColor : control.disabledBackgroundColor
        border.width: 2
        border.color: control.enabled ? control.borderColor : control.disabledBorderColor
    }
}
