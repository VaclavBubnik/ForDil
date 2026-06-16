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
    component ExerciseButton : IconTextButton {
        iconWidth: 30
        iconHeight: 30
        Layout.preferredWidth: 280 // 270
        Layout.preferredHeight: 50
        backgroundColor: "#26787A"
        borderColor: "#41B28E"
    }

    component InfoButton : IconTextButton {
        iconWidth: 25
        iconHeight: 25
        Layout.preferredWidth: 280
        Layout.preferredHeight: 40
    }

    component EndButton : TextButton {
        backgroundColor: "#26787A"
        borderColor: "#41B28E"
        width: 100
        height: 50
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
    }

    component NoButton : TextButton {
        backgroundColor: "#FF0000"
        borderColor: "#000000"
        textColor: "black"
        width: 80
        height: 50
    }

    component YesButton : TextButton {
        backgroundColor: "#26787A"
        borderColor: "#000000"
        width: 80
        height: 50
    }

    component ExitButton : IconTextButton {
        iconSource: "qrc:/images/ui_exit.png"
        width: 150
        height: 40
        backgroundColor: "#FF0000"
        borderColor: "#c90101"
    }
}
