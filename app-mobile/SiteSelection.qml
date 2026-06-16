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
import QtQuick.Dialogs
import ForDil
import "." 1.0

Rectangle {
    id: siteSelection

    property string system: "default"
    property bool discovery: false
    property string nextPage: "default"

    Component.onCompleted: {
        console.debug("SiteSelection completed: ", siteSelection.system,
                      siteSelection.discovery, siteSelection.nextPage);
    }

    // "Continue" button visibility
    property bool isContinueButtonVisible: false

    function updateContinueButtonVisibility() {
        var selectedSiteName = siteComboBox.model[siteComboBox.currentIndex].name;
        if (system === Backend.loadedSystem() &&
            discovery === Backend.loadedDiscoveryPart() &&
            selectedSiteName == Backend.loadedExerciseName())
        {
            isContinueButtonVisible = true;
        }
        else
        {
            isContinueButtonVisible =  false;
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateContinueButtonVisibility();
        }
    }

    // Use Column to arrange form rows vertically
    Column {
        width: 270
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 50
        spacing: 20

        // Menu title
        Rectangle {
            width: parent.width
            height: 30
            Text {
                text: qsTr("Select your site")
                font.pixelSize: 20
                font.bold: true
                color: "#26787A"

                width: parent.width
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Site name selection combo box
        ComboBox {
            id: siteComboBox
            width: 270
            height: 50

            contentItem: Text {
                text: siteComboBox.displayText
                color: "white"

                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                elide: Text.ElideRight
            }

            background: Rectangle {
                color: "#26787A"
                border.color: "#41B28E"
                radius: 7
            }

            // Flag to indicate if initialization is complete
            property bool fullyInitialized: false

            // keep selection by value, not index
            property string preservedName: ""

            textRole: "name"
            model: Backend.siteNamesModelData(siteSelection.system, siteSelection.discovery)

            function preserveSelection() {
                preservedName = currentText
            }

            function refreshAndRestoreSelection() {
                // prevent UI side-effects during rebuild
                fullyInitialized = false

                // IMPORTANT: if siteNamesModelData() returns a JS array, force refresh by reassigning
                model = Backend.siteNamesModelData(siteSelection.system, siteSelection.discovery)

                // restore selection by name (stable across sorting)
                if (preservedName.length > 0) {
                    for (var i = 0; i < model.length; ++i) {
                        if (model[i].name === preservedName) {
                            currentIndex = i
                            break
                        }
                    }
                }

                fullyInitialized = true
                siteSelection.updateContinueButtonVisibility()
            }

            // Update the combobox item with the currently loaded site name
            Component.onCompleted: {
                var siteName = Backend.loadedExerciseName()
                for (var i = 0; i < model.length; i++) {
                    var element = model[i];
                    if (element.name === siteName) {
                        currentIndex = i;
                        break;
                    }
                }

                fullyInitialized = true

                siteSelection.updateContinueButtonVisibility()
            }

            // Refresh "Continue" button visibility
            onCurrentIndexChanged: {
                if (fullyInitialized) {
                    siteSelection.updateContinueButtonVisibility()
                }
            }
        }

        // Start new exercise button
        RowLayout {
            IconTextButton {
                text: qsTr("Start new exercise")
                Layout.preferredWidth: 270
                Layout.preferredHeight: 40

                onClicked: {
                    var selectedSiteName = siteComboBox.model[siteComboBox.currentIndex].name;
                    var selectedPath = siteComboBox.model[siteComboBox.currentIndex].path;
                    console.debug("start selected site \"" + selectedSiteName + "\"");
                    console.debug("start selected path \"" + selectedPath + "\"");
                    Backend.loadExercise(selectedPath, selectedSiteName, siteSelection.system, siteSelection.discovery, true)
                    Globals.pageMain.pushPage(siteSelection.nextPage)
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }

        // Continue exercise button
        RowLayout {
            IconTextButton {
                text: qsTr("Continue exercise")
                Layout.preferredWidth: 270
                Layout.preferredHeight: 40

                visible: siteSelection.isContinueButtonVisible

                onClicked: {
                    var selectedSiteName = siteComboBox.model[siteComboBox.currentIndex].name;
                    console.debug("continue selected site \"" + selectedSiteName + "\"")
                    Backend.loadExercise("", selectedSiteName, siteSelection.system, siteSelection.discovery, false)
                    Globals.pageMain.pushPage(siteSelection.nextPage)
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }

        // Spacer
        Rectangle {
            width: 1
            height: 10
            color: "transparent"
        }

        // Check update button
        RowLayout {
            IconTextButton {
                visible: siteSelection.system === "Travailloscope"
                text: Backend.text("Check")
                secondaryText: Backend.text("if an update is available")
                iconSource: "qrc:/images/ui_update.png"
                Layout.preferredWidth: 270
                Layout.preferredHeight: 50
                onClicked: Qt.openUrlExternally("https://www.fordil.net/app-update")
            }

            Item {
                Layout.fillWidth: true
            }
        }

        // Import button
        RowLayout {
            IconTextButton {
                visible: siteSelection.system === "Marteloscope" && !siteSelection.discovery
                text: Backend.text("Select on your smartphone")
                secondaryText: Backend.text("Excel Form Marteloscope")
                iconSource: "qrc:/images/ui_import.png"
                Layout.preferredWidth: 270
                Layout.preferredHeight: 160
                onClicked: fileDialog.open()
            }

            IconTextButton {
                visible: siteSelection.system === "Travailloscope"
                text: Backend.text("Select on your smartphone")
                secondaryText: Backend.text("Excel Form Travailloscope")
                iconSource: "qrc:/images/ui_import.png"
                Layout.preferredWidth: 270
                Layout.preferredHeight: 160
                onClicked: fileDialog.open()
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    // Import dialog
    FileDialog {
        id: fileDialog
        title: Backend.text("Import") // "Select XLSX"
        nameFilters: ["XLSX files (*.xlsx)"]
        onAccepted: {
            // preserve current selection before backend modifies the data
            siteComboBox.preserveSelection()

            let msg = Backend.storeFile(fileDialog.currentFile)

            // rebuild items and restore selection
            siteComboBox.refreshAndRestoreSelection()

            // Show information message
            messageDialog.title = Backend.text("Import")
            messageDialog.text = msg
            messageDialog.open();
        }
    }

    // Message dialog
    MessageDialog {
        id: messageDialog
        buttons: MessageDialog.Ok
    }
}

