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

Rectangle {
    id: pageAbout

    Component.onCompleted: {
        console.debug("PageAbout completed");
    }

    ZoomableView {
        anchors.fill: parent
        anchors.margins: 10

        content: [
            // Use Column to arrange form rows vertically
            Column {
                width: parent.width
                height: childrenRect.height

                spacing: 10

                // Image
                Image {
                    source: "qrc:/images/ui_about.png"
                    fillMode: Image.PreserveAspectFit
                    height: pageAbout.height * 0.5
                    width: pageAbout.width
                    cache: true
                }

                // Spacer to push text to bottom
                Item {
                    width: parent.width
                    height: 20
                }

                // Version
                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 18
                    text: "ForDil © 2025–present <a href='https://mendelu.cz/'>MENDELU</a>"
                    textFormat: Text.RichText
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 14
                    text: qsTr("Built on") + " " + buildDate
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter

                    text: Backend.text("About: license")

                    font.pixelSize: 14
                    wrapMode: TextArea.Wrap
                    textFormat: TextArea.RichText
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }

                // 3rd party notice
                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                
                    text: Backend.text("About: 3rd party notice")

                    font.pixelSize: 14
                    wrapMode: TextArea.Wrap
                    textFormat: TextArea.RichText
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }
            }
        ]
    }
}
