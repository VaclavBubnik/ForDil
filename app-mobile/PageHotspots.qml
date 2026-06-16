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
    id: pageHotspots
    objectName: "PageHotspots"

    property var hotspotsModelData: []

    function refreshHotspotsModelData() {
        // Triggers update of hotspotsTable
        hotspotsModelData = Backend.hotspotsModelData();
    }

    Component.onCompleted: {
        Globals.pageHotspots = pageHotspots
        console.debug("PageHotspots completed");

        refreshHotspotsModelData(); // Initial load

        if (Backend.isIntroductionResultsVisible())
        {
            pedagogicalContents.dialogHeader.text = Backend.text("Results: Marteloscope Hotspots Introduction Header")
            pedagogicalContentsText.text = Backend.text("Results: Marteloscope Hotspots Introduction")
            pedagogicalContents.visible = true

            closeButton.showLabel = true
        }
    }

    function isBlockedByOverlay() {
        return pedagogicalContents.visible
    }

    // Results
    Rectangle {
        enabled: !pageHotspots.isBlockedByOverlay()

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.fill: parent
        anchors.margins: 20

        // Make the content scrollable
        ScrollView {
            anchors.fill: parent
            clip: true

            // Use Column to arrange charts vertically
            Column {
                id: hotspotsContent
                width: parent.width
                height: implicitHeight
                spacing: 10

                // Results
                Text {
                    id: hotspotsResultsText
                    text: qsTr("Results")
                }

                // Horizontal black line
                Rectangle {
                    id: hotspotsResultsTextLine
                    width: parent.width
                    height: 1
                    color: "black"
                }

                // Ranking
                RowLayout {
                    Text {
                        id: hotspotsRankingText
                        text: qsTr("Ranking")
                    }

                    Rectangle {
                        width: 50
                        height: 1
                        color: "transparent"
                    }

                    Rectangle {
                        id: hotspotsRankingScore
                        color: Backend.hotspotsRankingColor()
                        border.color: "black"
                        border.width: 1
                        Text {
                            id: hotspotsRankingScoreText
                            anchors.centerIn: parent
                            anchors.margins: 10
                            text: Backend.hotspotsRankingText()
                        }
                        implicitWidth: hotspotsRankingScoreText.implicitWidth + 20
                        implicitHeight: hotspotsRankingScoreText.implicitHeight + 20
                    }
                }

                // Horizontal black line
                Rectangle {
                    id: hotspotsRankingTextLine
                    width: 100
                    height: 1
                    color: "black"
                }

                // Score
                GridLayout {
                    id: hotspotsTable
                    columns: 4

                    // Reason
                    Repeater {
                        model: pageHotspots.hotspotsModelData
                        Text {
                            anchors.margins: 10
                            text: modelData.reasonTr

                            Layout.column: 0
                            Layout.row: index
                        }
                    }

                    // Score
                    Repeater {
                        model: pageHotspots.hotspotsModelData
                        Rectangle {
                            color: modelData.color

                            // Find max side to make it square
                            property int side: Math.max(scoreText.implicitWidth, scoreText.implicitHeight) + 10
                            width: side * Globals.bigScreenMultiplier
                            height: side * Globals.bigScreenMultiplier
                            radius: (side / 2) * Globals.bigScreenMultiplier // Perfect circle

                            Text {
                                id: scoreText
                                text: modelData.score
                                anchors.centerIn: parent
                            }

                            Layout.column: 2
                            Layout.row: index
                        }
                    }

                    // Details
                    Repeater {
                        model: pageHotspots.hotspotsModelData

                        Item {
                            width: 32
                            height: 32
                            Layout.leftMargin: 2
                            Layout.preferredWidth: 30 * Globals.bigScreenMultiplier
                            Layout.preferredHeight: 30 * Globals.bigScreenMultiplier
                            Layout.column: 3
                            Layout.row: index

                            Image {
                                id: hotspotImage
                                anchors.fill: parent
                                source: modelData.detailImageSource
                                fillMode: Image.PreserveAspectFit
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    Backend.setHotspotsReasonText(modelData.reason)
                                    Backend.resetZoom("mapImage")
                                    hotspotsMapView.visible = true
                                    hotspotsMapView.redraw()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Hotspots map
    property alias mapView: hotspotsMapView

    MapView {
        id: hotspotsMapView
        enabled: !pageHotspots.isBlockedByOverlay()
        anchors.fill: parent
        mapImageId: "mapImage"
        visible: false
    }

    // Dummy
    Text { id: treeId;       text: ""; visible: false; }
    Text { id: treeDBH;      text: ""; visible: false; }
    Text { id: treeSpecies;  text: ""; visible: false; }

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
        visible: hotspotsMapView.visible || pedagogicalContents.visible

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (pedagogicalContents.visible)
                {
                    pedagogicalContents.visible = false
                    hotspotsMapView.visible = true

                    if (Backend.isIntroductionResultsVisible())
                    {
                        Backend.setIntroductionResultsVisible(false)
                        hotspotsMapView.visible = false
                        closeButton.showLabel = false
                    }
                }
                else
                {
                    hotspotsMapView.visible = false
                }
            }
        }
    }
}
