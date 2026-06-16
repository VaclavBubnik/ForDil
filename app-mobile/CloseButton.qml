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

Item {
    id: control

    property bool showLabel: false

    width: 30 * Globals.bigScreenMultiplier
    height: 30 * Globals.bigScreenMultiplier

    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: 10 / Globals.bigScreenMultiplier

    Image {
        id: closeIcon
        anchors.fill: parent
        source: "qrc:/images/ui_close.png"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        text: qsTr("Close")
        color: "red"
        visible: control.showLabel

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: closeIcon.left
        anchors.rightMargin: 6
    }
}
