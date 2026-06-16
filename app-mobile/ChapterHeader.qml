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

Flow {
    id: root
    property string chapterImage: ""
    required property string headerText
    property string explanationHeader: ""
    required property string explanationKey

    property bool compassVisible: false
    property double compassAngle: 0

    default property alias extraContent: extraContainer.data

    width: parent.width

    // Number
    Item {
        visible: root.chapterImage !== ""

        width: 30
        height: 30

        Image {
            anchors.fill: parent
            source: root.chapterImage
            fillMode: Image.PreserveAspectFit
        }
    }

    // Spacer
    Item {
        visible: root.chapterImage !== ""
        width: 5
        height: 30
    }

    // Header
    Rectangle {
        width: textItem.implicitWidth
        height: 30
        color: "transparent"
        Text {
            id: textItem
            text: Backend.text(headerText)
            color: "#26787A"
            font.pixelSize: 20
        }
    }

    // Compass : Optional
    Item {
        visible: root.compassVisible
        width: 10
        height: 30
    }

    Item {
        visible: root.compassVisible

        width: 30
        height: 30

        Image {
            anchors.fill: parent
            source: "qrc:/images/ui_compass_arrow_n.png"
            fillMode: Image.PreserveAspectFit
            transform: Rotation {
                origin.x: 15
                origin.y: 15
                angle: -root.compassAngle
            }
        }
    }

    // Explanation : Optional
    Item {
        visible: root.explanationHeader !== ""
        width: 10
        height: 30
    }

    // Explanation
    Item {
        visible: root.explanationHeader !== ""

        width: 30
        height: 30

        Image {
            anchors.fill: parent
            source: "qrc:/images/ui_question.png"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                bus.showPedagogical(root.explanationKey, root.explanationHeader)
            }
        }
    }

    // Container for optional extra
    Column {
        id: extraContainer
        Layout.alignment: Qt.AlignTop
    }
}
