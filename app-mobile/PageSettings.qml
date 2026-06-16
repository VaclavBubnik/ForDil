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

Page {
    id: pageSettings

    Component.onCompleted: {
        console.debug("PageSettings completed");
    }

    // Dummy property to force re-evaluation of qsTr() bindings.
    property int languageRefresh: 0

    // Wrap the settings container in a ScrollView for scrolling when needed
    ScrollView {
        anchors.fill: parent
        anchors.topMargin: 40

        contentWidth: parent.width
        contentHeight: settingsFormContainer.implicitHeight

        // A wrapper Item to allow proper centering
        Item {
            width: parent.width    // Match ScrollView content width
            height: settingsFormContainer.implicitHeight

            // Use Column to arrange form rows vertically
            Column {
                id: settingsFormContainer
                width: 300
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                // Title
                Rectangle {
                    width: parent.width
                    height: 30
                    Text {
                        text: { pageSettings.languageRefresh; return qsTr("Settings") }
                        font.pixelSize: 20
                        font.bold: true
                        color: "#26787A"

                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // Language setting
                RowLayout {
                    width: parent.width
                    Layout.fillWidth: true

                    Label {
                        text: { pageSettings.languageRefresh; return qsTr("Language") }
                        Layout.alignment: Qt.AlignLeft
                    }

                    // Language selection
                    ComboBox {
                        id: languageComboBox
                        Layout.fillWidth: true

                        // Flag to indicate if initialization is complete
                        property bool fullyInitialized: false

                        // Hardcoded languages, check directory "translations"
                        model: ListModel {
                            ListElement { name: "English"; code: "en" }
                            // ListElement { name: "Español"; code: "es" }
                            ListElement { name: "Français"; code: "fr" }
                            ListElement { name: "Čeština"; code: "cs" }
                            // ListElement { name: "Deutsch"; code: "de" }
                        }

                        textRole: "name"

                        // Update the combobox item with the current language
                        Component.onCompleted: {
                            var languageCode = Backend.languageCode()
                            for (var i = 0; i < model.count; i++) {
                                var element = model.get(i);
                                if (element.code === languageCode) {
                                    currentIndex = i;
                                    break;
                                }
                            }

                            fullyInitialized = true
                        }

                        // Apply new language
                        onCurrentIndexChanged: {
                            if (fullyInitialized) {
                                var languageCode = model.get(currentIndex).code;
                                Backend.setLanguageByCode(languageCode)

                                // Refresh "Language" label text
                                pageSettings.languageRefresh++;
                                Globals.pageMain.refreshTr();
                            }
                        }
                    }
                }

                // Tree scale
                RowLayout {
                    width: parent.width
                    Layout.fillWidth: true

                    Label {
                        text: { pageSettings.languageRefresh; return qsTr("Tree scale"); }
                        Layout.alignment: Qt.AlignLeft
                    }

                    Slider {
                        id: treeScaleSlider
                        Layout.fillWidth: true
                        from: 1
                        to: 10
                        stepSize: 1
                        value: Backend.treeScale()  // initial value

                        onValueChanged: {
                            Backend.setTreeScale(value); // Update settings
                        }
                    }
                }

                // GPS
                RowLayout {
                    width: parent.width
                    Layout.fillWidth: true

                    Label {
                        text: { pageSettings.languageRefresh; return qsTr("GPS enabled"); }
                        Layout.alignment: Qt.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: gpsEnabledSwitch
                        text: ""
                        checked: Backend.isGpsEnabled
                        onCheckedChanged: {
                            Backend.setLocationAvailable(false);
                            Backend.setGpsEnabled(checked); // Update settings
                        }
                    }
                }

                // GPS
                RowLayout {
                    width: parent.width
                    Layout.fillWidth: true

                    Label {
                        text: { pageSettings.languageRefresh; return qsTr("Show GPS coordinates"); }
                        Layout.alignment: Qt.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: showGpsCoordinatesEnabledSwitch
                        text: ""
                        checked: Backend.isShowGpsCoordinatesEnabled
                        onCheckedChanged: {
                            Backend.setShowGpsCoordinatesEnabled(checked);
                        }
                    }
                }

                // Debug
                RowLayout {
                    visible: Backend.isDebugBuild
                    width: parent.width
                    Layout.fillWidth: true

                    Label {
                        text: { pageSettings.languageRefresh; return qsTr("Enable debug features"); }
                        Layout.alignment: Qt.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: enableDebugFeaturesSwitch
                        text: ""
                        checked: Backend.isDebugModeEnabled
                        onCheckedChanged: {
                            Backend.setDebugModeEnabled(checked); // Update settings
                        }
                    }
                }
            }
        }
    }
}
