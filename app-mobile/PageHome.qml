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
import QtQuick.Dialogs
import QtCore
import ForDil
import "." 1.0

Rectangle {
    id: pageHome

    Component.onCompleted: {
        console.debug("PageHome completed");

        if (Backend.isIntroductionAppVisible())
        {
            appIntroduction.visible = true
        }
    }

    function isBlockedByOverlay() {
        return appIntroduction.visible
    }

    // Page Layout
    ColumnLayout {
        enabled: !pageHome.isBlockedByOverlay()
        anchors.fill: parent
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true

            spacing: 10

            // Menu
            Text {
                text: qsTr("Menu")
                font.pixelSize: 20
                font.bold: true
                color: "#26787A"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            // spacer
            Item {
                Layout.preferredHeight: 20
            }

            // Exercise
            Components.ExerciseButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Discovery part")
                secondaryText: qsTr("Discover the concepts of SMCC")
                iconSource: "qrc:/images/ui_search.png"
                onClicked: {
                    Globals.pageMain.pushPage("PageSiteDiscovery.qml")
                }
            }

            Components.ExerciseButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Marteloscope")
                secondaryText: qsTr("Simulate thinning in SMCC")
                iconSource: "qrc:/images/ui_forest.png"
                onClicked: {
                    Globals.pageMain.pushPage("PageSiteMarteloscope.qml")
                }
            }

            Components.ExerciseButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Travailloscope")
                secondaryText: qsTr("Analyze regeneration")
                iconSource: "qrc:/images/ui_plant.png"
                onClicked: {
                    Globals.pageMain.pushPage("PageSiteTravailloscope.qml")
                }
            }

            // spacer
            Item {
                Layout.preferredHeight: 10
            }

            // About
            Components.InfoButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("About ForDil")
                iconSource: "qrc:/images/ui_info.png"
                onClicked: {
                    Qt.openUrlExternally("https://www.fordil.net/fr")
                }
            }

            Components.InfoButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("About the application")
                iconSource: "qrc:/images/ui_info.png"
                onClicked: {
                    Qt.openUrlExternally("https://www.fordil.net/application")
                }
            }

            // Booking
            Components.InfoButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Book your site")
                iconSource: "qrc:/images/ui_reservation.png"
                onClicked: {
                    Qt.openUrlExternally("https://www.fordil.net/application")
                }
            }

            // Glossary
            Components.InfoButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Glossary")
                iconSource: "qrc:/images/ui_export.png"
                onClicked: {
                    savePdfDialog.open()
                }
            }
        }
    } // Page Layout

    // Licence
    Text {
        text: qsTr("Licence")
        font.pixelSize: 18
        font.underline: true
        color: "#26787A"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 10

        MouseArea {
            enabled: !pageHome.isBlockedByOverlay()
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                Globals.pageMain.pushPage("PageAbout.qml")
            }
        }
    }

    // Logo
    Image {
        source: "qrc:/images/ui_logo_co_funded_by_eu.jpg"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        width: 70
        height: 70
        fillMode: Image.PreserveAspectFit
    }

    // Save PDF Dialog
    FileDialog {
        id: savePdfDialog
        title: Backend.text("Save File")
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF files (*.pdf)"]
        defaultSuffix: "pdf"
    
        // choose a sane default folder (app-writable)
        currentFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        // Preselect default file name
        selectedFile: currentFolder + "/" + Backend.glossaryPdfFileName()
    
        onAccepted: {
            // `selectedFile` is a url
            const err = Backend.copyGlossaryToUrl(selectedFile)
            if (err.length > 0) {
                messageDialog.title = Backend.text("Error")
                messageDialog.text = err
                messageDialog.open()
            }
        }
    }

    // Message Dialog
    MessageDialog {
        id: messageDialog
        buttons: MessageDialog.Ok
    }

    // Introduction
    DialogWindow {
        id: appIntroduction
        anchors.fill: parent
        visible: false

        dialogHeader.text: Backend.text("App: Introduction Header")

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    text: Backend.text("App: Introduction")
                    width: parent.width
                    height: implicitHeight
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }

                // Spacer
                Rectangle {
                    width: 1
                    height: 10
                    color: "transparent"
                }

                // Buttons
                Row {
                    TextButton {
                        text: qsTr("Continue")
                        onClicked: {
                            appIntroduction.visible = false;
                            Backend.setIntroductionAppVisible(false)
                        }
                    }
                }
            }
        ]
    }
}
