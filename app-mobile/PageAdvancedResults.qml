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

Rectangle {
    id: pageAdvancedResults

    property var _init: (function() {
        Backend.calculateTypologyLocal();
        Backend.calculateTypologyGlobal();
        Backend.calculateOverallAssessment();
        return 0
    })()

    Component.onCompleted: {
        Globals.pageAdvancedResults = pageAdvancedResults
        console.debug("PageAdvancedResults completed");

        if (Backend.isIntroductionAdvancedResultsVisible())
        {
            pedagogicalContents.dialogHeader.text = Backend.text("Results: Advanced Results Introduction Header")
            pedagogicalContentsText.text = Backend.text("Results: Advanced Results Introduction")
            pedagogicalContents.visible = true

            closeButton.showLabel = true
        }
    }

    function isBlockedByOverlay() {
        return pedagogicalContents.visible
    }

    // Local reusable components
    QtObject {
        id: bus
        signal showPedagogical(string key, string headerText)

        onShowPedagogical: function(key, headerText) {
            pedagogicalContents.visible = true
            pedagogicalContents.dialogHeader.text = Backend.text(headerText)
            pedagogicalContentsText.text = Backend.text(key)
        }
    }

    // -------------------------------------------------------------------------
    // 1
    component TypologyLocalBefore: Column {
        width: Globals.resultComponentAutoWidth() / 2

        property int spacerHeight: 0   // external control

        Item {
            width: parent.width
            height: parent.spacerHeight
        }

        ChapterHeader {
            headerText: "Results: Local Typology Before Header"
            explanationHeader: ""
            explanationKey: ""
            compassVisible: true
            compassAngle: Backend.typologyLocalResultsNorthAngle
        }

        Rectangle {
            width: parent.width
            height: parent.width * 1.7

            Image {
                id: typologyLocalBeforeImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: typologyLocalBeforeImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/typologyLocalBeforeImage"
            }
        }

        Item {
            width: parent.width
            height: 10
        }
    }

    component TypologyLocalAfter: Column {
        width: Globals.resultComponentAutoWidth() / 2

        property alias switchHeight: expertSwitch.height

        Row {
            Switch {
                id: expertSwitch
                text: Backend.text("Results: Text: Expert")
                width: implicitWidth
                height: 30

                contentItem: Text {
                    text: expertSwitch.text
                    color: "#26787A"
                    font: expertSwitch.font
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: expertSwitch.indicator.width + expertSwitch.spacing
                }

                Component.onCompleted: {
                    checked = Backend.typologyLocalResultsSettings === "Expert"
                }
                onCheckedChanged: {
                    let option = checked ? "Expert" : "User";
                    Backend.setTypologyLocalResultsSettings(option); // Update settings
                    typologyLocalAfterImage.redraw(); // Refresh data
                }
            }
        }

        ChapterHeader {
            headerText: "Results: Local Typology After Header"
            explanationHeader: ""
            explanationKey: ""
            compassVisible: true
            compassAngle: Backend.typologyLocalResultsNorthAngle
        }

        Rectangle {
            width: parent.width
            height: parent.width * 1.7

            Image {
                id: typologyLocalAfterImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: typologyLocalAfterImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/typologyLocalAfterImage"

                function redraw() {
                    typologyLocalAfterImage.source = ""
                    typologyLocalAfterImage.source = typologyLocalAfterImage.imageSource
                }
            }
        }

        Item {
            width: parent.width
            height: 10
        }
    }

    component TypologyLocal: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_01.png"
            headerText: "Results: Local Typology Header"
            explanationHeader: "Results: Local Typology Explanation Header"
            explanationKey: "Results: Local Typology Explanation"
        }

        RowLayout {
            spacing: 0

            TypologyLocalBefore {
                id: typologyLocalBefore
                spacerHeight: 0
                Binding {
                    target: typologyLocalBefore
                    property: "spacerHeight"
                    value: typologyLocalAfter.switchHeight
                }
            }

            TypologyLocalAfter {
                id: typologyLocalAfter
            }
        }

        Item {
            width: parent.width
            height: 10
        }
    }

    // 2
    component TypologyGlobal: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_02.png"
            headerText: "Results: Global Typology Header"
            explanationHeader: "Results: Global Typology Explanation Header"
            explanationKey: "Results: Global Typology Explanation"

            Switch {
                id: expertSwitch
                text: Backend.text("Results: Text: +10 years")
                width: implicitWidth
                height: 30

                contentItem: Text {
                    text: expertSwitch.text
                    color: "#26787A"
                    font: expertSwitch.font
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: expertSwitch.indicator.width + expertSwitch.spacing
                }

                Component.onCompleted: {
                    checked = Backend.typologyGlobalResultsSettings === "10 years"
                }
                onCheckedChanged: {
                    let option = checked ? "10 years" : "0 years";
                    Backend.setTypologyGlobalResultsSettings(option); // Update settings
                    typologyGlobalImage.redraw(); // Refresh data
                }
            }
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: typologyGlobalImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                sourceSize: Qt.size(width, height)
                source: typologyGlobalImage.imageSource
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/typologyGlobalImage"

                function redraw() {
                    typologyGlobalImage.source = "";
                    typologyGlobalImage.source = typologyGlobalImage.imageSource;
                }
            }
        }

        Item {
            width: parent.width
            height: 10
        }
    }

    // 3
    component OverallAssessment: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_03.png"
            headerText: "Results: Overall CCF assessment Header"
            explanationHeader: "Results: Overall CCF assessment Explanation Header"
            explanationKey: "Results: Overall CCF assessment Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width / 2

            Image {
                id: overallAssessmentImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: overallAssessmentImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/overallAssessmentImage"
            }
        }

        Item {
            width: parent.width
            height: 10
        }
    }
    // -------------------------------------------------------------------------

    // Results
    ZoomableView {
        enabled: !pageAdvancedResults.isBlockedByOverlay()

        anchors.fill: parent
        anchors.margins: 5
        anchors.topMargin: 10

        content: [
            Column {
                id: contentColumn
                width: parent.width
                spacing: 5

                RowLayout {
                    IconTextButton {
                        text: Backend.text("Download")
                        secondaryText: Backend.text("All your results in PDF")
                        iconSource: "qrc:/images/ui_export.png"
                        Layout.preferredWidth: 260
                        Layout.preferredHeight: 50
                        onClicked: savePdfDialog.open()
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }

                Flow {
                    width: contentColumn.width
                    TypologyLocal { id: typologyLocal }
                    TypologyGlobal { id: typologyGlobal }
                    OverallAssessment { id: overallAssessment }
                }

                RowLayout {
                    IconTextButton {
                        text: Backend.text("Your feedback")
                        secondaryText: Backend.text("To improve the application")
                        Layout.preferredWidth: 260
                        Layout.preferredHeight: 50
                        onClicked: Qt.openUrlExternally("https://www.fordil.net/users-feedback")
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        ]
    }

    // Save PDF dialog
    FileDialog {
        id: savePdfDialog
        title: Backend.text("Export to PDF")
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF files (*.pdf)"]
        defaultSuffix: "pdf"

        // choose a sane default folder (app-writable)
        currentFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

        onAccepted: {
            // file is a url (may be file:///... or content://...)
            const err = Backend.exportMarteloscopeResultsToPdf(selectedFile)

            // Show information message
            if (err.length > 0) {
                messageDialog.title = Backend.text("Error")
                messageDialog.text = msg
                messageDialog.open();
            }
            else
            {
                messageDialog.title = Backend.text("Export to PDF")
                messageDialog.text = Backend.text("File saved successfully")
                messageDialog.open();
            }
        }
    }

    // Message Dialog
    MessageDialog {
        id: messageDialog
        buttons: MessageDialog.Ok
    }

    // Pedagogical contents
    DialogWindow {
        id: pedagogicalContents
        dialogHeader.text: ""
        hasClose: true
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: pedagogicalContentsText
                    text: ""
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
            }
        ]
    }

    // Close button
    CloseButton {
        id: closeButton
        visible: pedagogicalContents.visible

        MouseArea {
            anchors.fill: parent
            onClicked: {
                pedagogicalContents.visible = false
                closeButton.showLabel = false
                Backend.setIntroductionAdvancedResultsVisible(false)
            }
        }
    }
}
