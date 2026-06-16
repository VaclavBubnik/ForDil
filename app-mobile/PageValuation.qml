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
    id: pageValuation
    objectName: "PageValuation"

    property var valuationModelData: []

    function refreshValuationModelData() {
        // Triggers update of valuationTable
        Backend.travailloscopeCalculateScore();
        valuationModelData = Backend.valuationModelData();
        totalScoreRectangle.color = Backend.valuationRankingColor()
        totalScoreText.text = Backend.valuationRankingText()
    }

    Component.onCompleted: {
        Globals.pageValuation = pageValuation
        console.debug("PageValuation completed");

        refreshValuationModelData(); // Initial load

        if (Backend.isIntroductionResultsVisible())
        {
            pedagogicalContents.dialogHeader.text = Backend.text("Results: Travailloscope Valuation Introduction Header")
            pedagogicalContentsText.text = Backend.text("Results: Travailloscope Valuation Introduction")
            pedagogicalContents.visible = true

            closeButton.showLabel = true
        }
    }

    function isBlockedByOverlay() {
        return pedagogicalContents.visible
    }

    // Results
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.fill: parent
        anchors.margins: 20

        enabled: !pageValuation.isBlockedByOverlay()

        // Make the content scrollable
        ScrollView {
            anchors.fill: parent
            clip: true

            // Use Column to arrange charts vertically
            Column {
                id: valuationContent
                width: parent.width
                height: implicitHeight
                spacing: 10

                // Total score
                RowLayout {
                    Text {
                        text: Backend.text("Score")
                    }

                    Rectangle {
                        Layout.preferredWidth: 25
                        Layout.preferredHeight: 1
                        color: "transparent"
                    }

                    Rectangle {
                        id: totalScoreRectangle
                        color: "transparent"
                        border.color: "black"
                        border.width: 1
                        Text {
                            id: totalScoreText
                            anchors.centerIn: parent
                            anchors.margins: 10
                            text: "0"
                        }
                        implicitWidth: totalScoreText.implicitWidth + 20
                        implicitHeight: totalScoreText.implicitHeight + 20
                    }
                }

                // Spacer
                Rectangle {
                    width: 1
                    height: 10
                    color: "transparent"
                }

                // Score list
                GridLayout {
                    id: valuationTable
                    columns: 3

                    // Spot id
                    Repeater {
                        model: pageValuation.valuationModelData
                        Text {
                            anchors.margins: 10
                            text: modelData.label

                            Layout.column: 0
                            Layout.row: index
                        }
                    }

                    // Score per spot
                    Repeater {
                        model: pageValuation.valuationModelData
                        Text {
                            anchors.margins: 10
                            text: modelData.score
                            color: modelData.color

                            Layout.leftMargin: 10

                            Layout.column: 1
                            Layout.row: index
                        }
                    }

                    // Show map
                    Item {
                        Layout.preferredWidth: 80
                        implicitHeight: column2.implicitHeight

                        Layout.leftMargin: 30

                        Layout.column: 2
                        Layout.row: 0
                        Layout.rowSpan: 10
                        Layout.alignment: Qt.AlignVCenter

                        Column {
                            id: column2
                            anchors.fill: parent
                            spacing: 6

                            Image {
                                width: parent.width
                                source: "qrc:/images/ui_location_on_map.png"
                                fillMode: Image.PreserveAspectFit
                            }

                            Text {
                                width: parent.width
                                text: qsTr("Show on map")
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                Backend.setValuationReasonText("Valuation")
                                Backend.resetZoom("mapImage")
                                valuationMapView.visible = true
                                valuationMapView.redraw()
                            }
                        }
                    }
                }

                // Spacer
                Rectangle {
                    width: 1
                    height: 10
                    color: "transparent"
                }

                // Buttons
                RowLayout {
                    IconTextButton {
                        text: Backend.text("Export to PDF")
                        iconSource: "qrc:/images/ui_export.png"
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 40
                        onClicked: savePdfDialog.open()
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
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
            const err = Backend.exportTravailloscopeQuizToPdf(selectedFile)

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

    // Map
    property alias mapView: valuationMapView

    MapView {
        id: valuationMapView
        enabled: !pageValuation.isBlockedByOverlay()
        anchors.fill: parent
        mapImageId: "mapImage"
        visible: false
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
        visible: valuationMapView.visible || pedagogicalContents.visible

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (pedagogicalContents.visible)
                {
                    pedagogicalContents.visible = false
                    valuationMapView.visible = true

                    if (Backend.isIntroductionResultsVisible())
                    {
                        Backend.setIntroductionResultsVisible(false)
                        valuationMapView.visible = false
                        closeButton.showLabel = false
                    }
                }
                else
                {
                    valuationMapView.visible = false
                }
            }
        }
    }
}
