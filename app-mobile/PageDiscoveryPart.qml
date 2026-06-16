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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ForDil
import "." 1.0

Rectangle {
    id: pageDiscoveryPart
    objectName: "PageDiscoveryPart"

    Component.onCompleted: {
        Globals.pageDiscoveryPart = pageDiscoveryPart;
        console.debug("PageDiscoveryPart completed");

        if (Backend.isIntroductionSiteVisible())
        {
            pageDiscoveryPart.showDiscoveryState();
        }

        if (Backend.discoveryHasNextState())
        {
            stopTextId.text = Backend.discoveryStopMessage();
        }
        else
        {
            stopTextId.text = "";
            startQuizButton.visible = Backend.discoveryHasQuestions();
        }
    }

    // Functions
    function showDiscoveryState()
    {
        explanation.dialogHeader.text = Backend.discoveryHeader();
        explanationText.text = Backend.discoveryText();
        explanation.visible = true;
        explanation.resetView();
    }

    function nextDiscoveryState()
    {
        if (!Backend.discoveryHasNextState())
        {
            explanation.visible = false;
            startQuizButton.visible = Backend.discoveryHasQuestions();
            Backend.discoveryGoToTree();
            mapView.redraw();
            return;
        }

        Backend.discoveryNextState();

        const stopId = Backend.discoveryStop();
        if (stopId > 0)
        {
            explanation.visible = false;
            goToTreeDialog.text = Backend.discoveryStopMessage()
            stopTextId.text = goToTreeDialog.text;
            goToTreeDialog.open();
            Backend.discoveryGoToTree();
        }
        else
        {
            stopTextId.text = "";
            showDiscoveryState();
        }

        mapView.redraw();
    }

    function resetCheckBoxes(repeater) {
        for (let i = 0; i < repeater.count; ++i) {
            const wrapper = repeater.itemAt(i);
            if (wrapper && wrapper.children.length > 0) {
                const citem = wrapper.children[0]; // the Column
                const ritem = citem.children[0];   // the Row
                const cb = ritem.children[0];      // the CheckBox
                cb.checked = false;
            }
        }
    }

    function isBlockedByOverlay() {
        return explanation.visible ||
               question1.visible ||
               answer1.visible ||
               wellDone.visible
    }

    // Map
    property alias mapView: discoveryMapView

    MapView {
        id: discoveryMapView
        enabled: !pageDiscoveryPart.isBlockedByOverlay()
        anchors.fill: parent
        mapImageId: "mapImage"
    }

    // Information about current stop
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10

        color: Qt.rgba(1, 1, 1, 0.85)

        implicitWidth: informationGrid.implicitWidth + 16
        implicitHeight: informationGrid.implicitHeight + 16

        GridLayout {
            id: informationGrid
            columns: 1
            rowSpacing: 6
            columnSpacing: 12
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 8

            Text { id: stopTextId; text: ""; Layout.alignment: Qt.AlignRight }
        }
    }

    // Start Quiz button
    Components.EndButton {
        id: startQuizButton
        text: qsTr("Start Quiz")
        width: 130
        enabled: !pageDiscoveryPart.isBlockedByOverlay()
        visible: false
        onClicked: {
            Backend.discoveryResetQuiz(); 
            question1.reload();
            question1.visible = true;
        }
    }

    // Go in front of tree message
    CustomPopup {
        id: goToTreeDialog

        text: "";

        buttons: [
            TextButton {
                text: qsTr("Ok")
                onClicked: {
                    goToTreeDialog.close();
                }
            }
        ]
    }

    // Explanation contents
    DialogWindow {
        id: explanation
        dialogHeader.text: ""
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: explanationText
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
                    height: 30
                    color: "transparent"
                }

                // Buttons
                Row {
                    TextButton {
                        text: qsTr("Next")
                        onClicked: {
                            Backend.setIntroductionSiteVisible(false)
                            pageDiscoveryPart.nextDiscoveryState()
                        }
                    }

                    Rectangle {
                        width: 20
                        height: 1
                        color: "transparent"
                    }

                    TextButton {
                        text: qsTr("Back to map")
                        visible: stopTextId.text !== "" && !Backend.isIntroductionSiteVisible()
                        onClicked: {
                            Backend.setIntroductionSiteVisible(false)
                            explanation.visible = false;
                        }
                    }
                }
            }
        ]
    }

    // Question
    DialogWindow {
        id: question1
        dialogHeader.text: ""
        visible: false

        property var checkBoxData1: []  // Store checkbox data

        function reload() {
            const questionId = Backend.discoveryQuestion();
            checkBoxData1 = Backend.discoveryCheckBoxItems(questionId);
            pageDiscoveryPart.resetCheckBoxes(checkBoxRepeater1);
            dialogHeader.text = Backend.discoveryQuestionHeader(questionId);
            question1Text.text = Backend.discoveryQuestionText(questionId);
        }

        content: [
            Column {
                id: question1Column
                width: parent.width
                height: childrenRect.height

                // Property to track which checkbox is selected (-1 = none)
                property int selectedIndex: -1

                // Dynamically load checkboxes based on a string list from C++
                Repeater {
                    id: checkBoxRepeater1
                    model: question1.checkBoxData1 // This should be set from C++

                    Item {
                        required property var modelData
                        required property int index

                        width: parent.width
                        height: checkColumn1.implicitHeight + 8  // Add some padding

                        Column {
                            id: checkColumn1
                            width: parent.width
                            spacing: 6

                            // Checkbox with answer text
                            Row {
                                id: checkRow1
                                width: parent.width
                                spacing: 6

                                // Checkbox indicator without text
                                CheckBox {
                                    id: checkBox1
                                    // checked: false
                                    //checked: question1Column.selectedIndex === index

                                    Binding {
                                        target: checkBox1
                                        property: "checked"
                                        value: question1Column.selectedIndex === index
                                    }

                                    onClicked: {
                                        if (question1Column.selectedIndex === index) {
                                            // unselect
                                            question1Column.selectedIndex = -1
                                        } else {
                                            // select
                                            question1Column.selectedIndex = index
                                        }
                                    }
                                }

                                // Label showing checkbox text that wraps
                                Label {
                                    text: modelData.text
                                    font.pixelSize: 16
                                    wrapMode: Text.WordWrap
                                    width: parent.width - checkBox1.width - checkRow1.spacing - 50
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            // Answer image is below on extra row
                            Text {
                                text: modelData.image
                                width: parent.width
                                textFormat: Text.RichText
                            }
                        } // Column
                    }
                }

                Text {
                    id: question1Text
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

                // Row of buttons: Validate
                Row {
                    TextButton {
                        text: qsTr("Validate")
                        enabled: question1Column.selectedIndex !== -1   // active only when a checkbox is selected
                        onClicked: {
                            question1Column.selectedIndex = -1
                            question1.visible = false;
                            answer1.reload();
                            answer1.visible = true;
                        }
                    }
                }
            }
        ]
    }

    // Answer
    DialogWindow {
        id: answer1
        dialogHeader.text: ""
        visible: false

        function reload() {
            const questionId = Backend.discoveryQuestion();
            dialogHeader.text = Backend.discoveryAnswerHeader(questionId);
            answer1Text.text = Backend.discoveryAnswerText(questionId);
        }

        content: [
            Column {
                id: answer1Column
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: answer1Text
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

                // Buttons
                Row {
                    TextButton {
                        text: qsTr("Next")
                        onClicked: {
                            answer1.visible = false

                            if (Backend.discoveryHasNextQuestion())
                            {
                                Backend.discoveryNextQuestion();
                                question1.reload();
                                question1.visible = true;
                            }
                            else
                            {
                                wellDone.visible = true;
                            }
                        }
                    }
                }
            }
        ]
    }

    // Well done
    DialogWindow {
        id: wellDone
        dialogHeader.text: Backend.text("Discovery: End Message Header")
        visible: false

        content: [
            Column {
                id: wellDoneColumn
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: wellDoneText
                    text: Backend.text("Discovery: End Message")
                    width: parent.width
                    height: implicitHeight
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
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
                        text: qsTr("End")
                        onClicked: {
                            wellDone.visible = false;
                            Backend.setExerciseFinished(true)
                            Globals.pageMain.pushPage("PageHome.qml")
                        }
                    }
                }
            }
        ]
    }
}
