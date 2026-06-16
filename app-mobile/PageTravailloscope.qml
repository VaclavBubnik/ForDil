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
import "." 1.0

Rectangle {
    id: pageTravailloscope
    objectName: "PageTravailloscope"

    property int contentMargins: 0

    Component.onCompleted: {
        Globals.pageTravailloscope = pageTravailloscope
        console.debug("PageTravailloscope completed");

        if (Backend.isIntroductionSiteVisible())
        {
            introductionScreen.dialogHeader.text = Backend.introductionSiteHeader()
            introductionText.text = Backend.introductionSiteBody()
            introductionScreen.visible = true
        }
    }

    // Functions
    function isQuizVisible() {
        return question1.visible ||
               answer1.visible ||
               question2.visible ||
               answer2.visible ||
               question3.visible ||
               answer3.visible ||
               question4.visible ||
               answer4.visible
    }

    function isBlockedByOverlay() {
        return introductionScreen.visible || isQuizVisible()
    }

    function closeQuiz() {
        question1.visible = false
        answer1.visible = false
        question2.visible = false
        answer2.visible = false
        question3.visible = false
        answer3.visible = false
        question4.visible = false
        answer4.visible = false
    }

    function resetCheckBoxes(repeater) {
        for (let i = 0; i < repeater.count; ++i) {
            const wrapper = repeater.itemAt(i);
            if (wrapper && wrapper.children.length > 0) {
                const citem = wrapper.children[0]; // the Row
                const cb = citem.children[0];      // the CheckBox
                cb.checked = false;
            }
        }
    }

    function startQuiz() {
        console.debug("start quiz");
        Backend.clearTravailoscopeAnswers();
        question1.checkBoxData1 = Backend.travailoscopeCheckBoxItems(1);
        question2.checkBoxData2 = Backend.travailoscopeCheckBoxItems(2);
        question3.checkBoxData3 = Backend.travailoscopeCheckBoxItems(3);
        question4.checkBoxData4 = Backend.travailoscopeCheckBoxItems(4);
        question1.visible = true;
    }

    function getSelectedIds(repeater, modelIdPropertyName) {
        let selected = [];

        for (let i = 0; i < repeater.count; ++i) {
            const wrapper = repeater.itemAt(i);
            if (!wrapper || wrapper.children.length === 0)
                continue;

            const citem = wrapper.children[0];
            const cb = citem.children[0]; // assumes CheckBox is the first child

            if (cb && cb.checked) {
                // Use model property name dynamically
                const id = repeater.model[i][modelIdPropertyName];
                selected.push(id);
            }
        }

        return selected;
    }

    // Map
    property alias mapView: spotSelectionMapView

    MapView {
        id: spotSelectionMapView
        enabled: !pageTravailloscope.isBlockedByOverlay()
        anchors.fill: parent
        mapImageId: "mapImage"
    }

    // End exercise button
    Components.EndButton {
        text: qsTr("End")
        enabled: !pageTravailloscope.isBlockedByOverlay()
        visible: !Backend.isExerciseFinished
        onClicked: {
            endExerciseConfirmationDialog.open();
        }
    }

    // Are you sure?
    CustomPopup {
        id: endExerciseConfirmationDialog

        title: qsTr("Confirm action")
        text: qsTr("Are You sure to end the exercise?");

        buttons: [
            Components.NoButton {
                text: qsTr("No")
                onClicked: {
                    endExerciseConfirmationDialog.close();
                }
            },

            Components.YesButton {
                text: qsTr("Yes")
                onClicked: {
                    endExerciseConfirmationDialog.close();
                    Backend.setExerciseFinished(true)
                    Globals.pageMain.pushPage("PageValuation.qml")
                }
            }
        ]
    }

    // Question 1
    DialogWindow {
        id: question1
        dialogHeader.text: Backend.travailloscopeQuestionHeader(1)
        hasClose: true
        visible: false

        property var checkBoxData1: []  // Store checkbox data

        // When question becomes visible, reset all CheckBoxes
        onVisibleChanged: {
            if (visible) {
                pageTravailloscope.resetCheckBoxes(checkBoxRepeater1)
                question1Column.selectedCount = 0
            }
        }

        content: [
            Column {
                id: question1Column
                width: parent.width
                height: childrenRect.height

                // Property to track if a checkbox is selected (0 = none)
                property int selectedCount: 0

                // Dynamically load checkboxes based on a string list from C++
                Repeater {
                    id: checkBoxRepeater1
                    model: question1.checkBoxData1 // This should be set from C++

                    Item {
                        width: parent.width
                        height: checkRow1.implicitHeight + 8  // Add some padding
                    
                        Row {
                            id: checkRow1
                            width: parent.width
                            spacing: 6
                    
                            // Checkbox indicator without text
                            CheckBox {
                                id: checkBox1
                                checked: false

                                onClicked: {
                                    const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater1, "id");
                                    question1Column.selectedCount = selected.length
                                }
                            }
                    
                            // Label showing checkbox text that wraps
                            Label {
                                text: modelData.text
                                wrapMode: Text.WordWrap
                                width: parent.width - checkBox1.width - checkRow1.spacing - 50
                                verticalAlignment: Text.AlignVCenter
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

                // Row of buttons: Clear and Validate
                Flow {
                    spacing: 10 // Space between buttons
                    width: parent.width

                    TextButton {
                        text: qsTr("Clear")
                        onClicked: {
                            pageTravailloscope.resetCheckBoxes(checkBoxRepeater1)
                            question1Column.selectedCount = 0
                        }
                    }

                    TextButton {
                        text: qsTr("Validate")
                        enabled: question1Column.selectedCount !== 0 // active only when a checkbox is selected
                        onClicked: {
                            // Get list of selected checkboxes
                            const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater1, "id");

                            // Send list of checked items to C++ as QStringList
                            Backend.setTravailloscopeAnswer(1, selected);

                            // The first answer must be correct to continue to the next question
                            if (Backend.travailloscopeScore(1) === 2)
                            {
                                question1.visible = false;
                                answer1Text.text = Backend.travailoscopeAnswerText(1)
                                answer1.visible = true;
                            }
                            else
                            {
                                tryAgainDialog.open()
                            }
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // A1
    DialogWindow {
        id: answer1
        dialogHeader.text: ""
        hasClose: true
        visible: false

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
                Flow {
                    spacing: 10
                    width: parent.width

                    TextButton {
                        text: qsTr("Next question")
                        onClicked: {
                            answer1.visible = false

                            const last = Backend.isTravailloscopeSpotFinished()
                            if (last)
                            {
                                if (mapView)
                                {
                                    mapView.redraw()
                                }
                            }
                            else
                            {
                                question2.visible = true
                            }
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // Question 2: Types of targeted interventions
    DialogWindow {
        id: question2
        dialogHeader.text: Backend.travailloscopeQuestionHeader(2)
        hasClose: true
        visible: false

        property var checkBoxData2: []  // Store checkbox data

        // When question becomes visible, reset all CheckBoxes
        onVisibleChanged: {
            if (visible) {
                pageTravailloscope.resetCheckBoxes(checkBoxRepeater2)
                question2Column.selectedCount = 0
            }
        }

        content: [
            Column {
                id: question2Column
                width: parent.width
                height: childrenRect.height

                // Property to track if a checkbox is selected (0 = none)
                property int selectedCount: 0

                // Dynamically load checkboxes based on a string list from C++
                Repeater {
                    id: checkBoxRepeater2
                    model: question2.checkBoxData2 // This should be set from C++

                    Item {
                        width: parent.width
                        height: checkRow2.implicitHeight + 8  // Add some padding
                    
                        Row {
                            id: checkRow2
                            width: parent.width
                            spacing: 6
                    
                            // Checkbox indicator without text
                            CheckBox {
                                id: checkBox2
                                checked: false

                                onClicked: {
                                    const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater2, "id");
                                    question2Column.selectedCount = selected.length
                                }
                            }
                    
                            // Label showing checkbox text that wraps
                            Label {
                                text: modelData.text
                                wrapMode: Text.WordWrap
                                width: parent.width - checkBox2.width - checkRow2.spacing - 50
                                verticalAlignment: Text.AlignVCenter
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

                // Row of buttons: Clear and Validate
                Flow {
                    spacing: 10 // Space between buttons
                    width: parent.width

                    TextButton {
                        text: qsTr("Clear")
                        onClicked: {
                            pageTravailloscope.resetCheckBoxes(checkBoxRepeater2)
                            question2Column.selectedCount = 0
                        }
                    }

                    TextButton {
                        text: qsTr("Validate")
                        enabled: question2Column.selectedCount !== 0 // active only when a checkbox is selected
                        onClicked: {
                            // Get list of selected checkboxes
                            const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater2, "id");

                            // Send list of checked items to C++ as QStringList
                            Backend.setTravailloscopeAnswer(2, selected);

                            // The second answer must be correct to continue to the next question
                            if (Backend.travailloscopeScore(2) === 2)
                            {
                                question2.visible = false;
                                answer2Text.text = Backend.travailoscopeAnswerText(2)
                                answer2.visible = true;
                            }
                            else
                            {
                                tryAgainDialog.open()
                            }
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // A2
    DialogWindow {
        id: answer2
        dialogHeader.text: ""
        hasClose: true
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: answer2Text
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
                Flow {
                    spacing: 10
                    width: parent.width

                    TextButton {
                        text: qsTr("Next question")
                        onClicked: {
                            answer2.visible = false

                            const last = Backend.isTravailloscopeSpotFinished()
                            if (last)
                            {
                                if (mapView)
                                {
                                    mapView.redraw()
                                }
                            }
                            else
                            {
                                question3.visible = true
                            }
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // Question 3
    DialogWindow {
        id: question3
        dialogHeader.text: Backend.travailloscopeQuestionHeader(3)
        hasClose: true
        visible: false

        property var checkBoxData3: []  // Store checkbox data

        // When question becomes visible, reset all CheckBoxes
        onVisibleChanged: {
            if (visible) {
                pageTravailloscope.resetCheckBoxes(checkBoxRepeater3)
                question3Column.selectedCount = 0
            }
        }

        content: [
            Column {
                id: question3Column
                width: parent.width
                height: childrenRect.height

                // Property to track if a checkbox is selected (0 = none)
                property int selectedCount: 0

                // Dynamically load checkboxes based on a string list from C++
                Repeater {
                    id: checkBoxRepeater3
                    model: question3.checkBoxData3 // This should be set from C++

                    Item {
                        width: parent.width
                        height: checkRow3.implicitHeight + 8  // Add some padding
                    
                        Row {
                            id: checkRow3
                            width: parent.width
                            spacing: 6
                    
                            // Checkbox indicator without text
                            CheckBox {
                                id: checkBox3
                                checked: false

                                onClicked: {
                                    const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater3, "id");
                                    question3Column.selectedCount = selected.length
                                }
                            }
                    
                            // Label showing checkbox text that wraps
                            Label {
                                text: modelData.text
                                wrapMode: Text.WordWrap
                                width: parent.width - checkBox3.width - checkRow3.spacing - 50
                                verticalAlignment: Text.AlignVCenter
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

                // Row of buttons: Clear and Validate
                Flow {
                    spacing: 10 // Space between buttons
                    width: parent.width

                    TextButton {
                        text: qsTr("Clear")
                        onClicked: {
                            pageTravailloscope.resetCheckBoxes(checkBoxRepeater3)
                            question3Column.selectedCount = 0
                        }
                    }

                    TextButton {
                        text: qsTr("Validate")
                        enabled: question3Column.selectedCount !== 0 // active only when a checkbox is selected
                        onClicked: {
                            // Get list of selected checkboxes
                            const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater3, "id");

                            // Send list of checked items to C++ as QStringList
                            Backend.setTravailloscopeAnswer(3, selected);

                            // Show answer
                            question3.visible = false;
                            answer3Text.text = Backend.travailoscopeAnswerText(3)
                            answer3.visible = true;
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // A3
    DialogWindow {
        id: answer3
        dialogHeader.text: ""
        hasClose: true
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: answer3Text
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
                Flow {
                    spacing: 10
                    width: parent.width

                    TextButton {
                        text: qsTr("Next question")
                        onClicked: {
                            answer3.visible = false

                            const last = Backend.isTravailloscopeSpotFinished()
                            if (last)
                            {
                                if (mapView)
                                {
                                    mapView.redraw()
                                }
                            }
                            else
                            {
                                question4.visible = true
                            }
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // Question 4
    DialogWindow {
        id: question4
        dialogHeader.text: Backend.travailloscopeQuestionHeader(4)
        hasClose: true
        visible: false

        property var checkBoxData4: []  // Store checkbox data

        // When question becomes visible, reset all CheckBoxes
        onVisibleChanged: {
            if (visible) {
                pageTravailloscope.resetCheckBoxes(checkBoxRepeater4)
                question4Column.selectedCount = 0
            }
        }

        content: [
            Column {
                id: question4Column
                width: parent.width
                height: childrenRect.height

                // Property to track if a checkbox is selected (0 = none)
                property int selectedCount: 0

                // Dynamically load checkboxes based on a string list from C++
                Repeater {
                    id: checkBoxRepeater4
                    model: question4.checkBoxData4 // This should be set from C++

                    Item {
                        width: parent.width
                        height: checkRow4.implicitHeight + 8  // Add some padding
                    
                        Row {
                            id: checkRow4
                            width: parent.width
                            spacing: 6
                    
                            // Checkbox indicator without text
                            CheckBox {
                                id: checkBox4
                                checked: false

                                onClicked: {
                                    const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater4, "id");
                                    question4Column.selectedCount = selected.length
                                }
                            }
                    
                            // Label showing checkbox text that wraps
                            Label {
                                text: modelData.text
                                wrapMode: Text.WordWrap
                                width: parent.width - checkBox4.width - checkRow4.spacing - 50
                                verticalAlignment: Text.AlignVCenter
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

                // Row of buttons: Clear and Validate
                Flow {
                    spacing: 10
                    width: parent.width

                    TextButton {
                        text: qsTr("Clear")
                        onClicked: {
                            pageTravailloscope.resetCheckBoxes(checkBoxRepeater4)
                            question4Column.selectedCount = 0
                        }
                    }

                    TextButton {
                        text: qsTr("Validate")
                        enabled: question4Column.selectedCount !== 0 // active only when a checkbox is selected
                        onClicked: {
                            // Get list of selected checkboxes
                            const selected = pageTravailloscope.getSelectedIds(checkBoxRepeater4, "id");

                            // Send list of checked items to C++ as QStringList
                            Backend.setTravailloscopeAnswer(4, selected);

                            // Show answer
                            question4.visible = false;
                            answer4Text.text = Backend.travailoscopeAnswerText(4)
                            answer4.visible = true;
                        }
                    }

                    Components.ExitButton {
                        text: qsTr("Exit")
                        onClicked: pageTravailloscope.closeQuiz()
                    }
                }
            }
        ]
    }

    // A4
    DialogWindow {
        id: answer4
        dialogHeader.text: ""
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: answer4Text
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
                        text: qsTr("Back to the map")
                        onClicked: {
                            // Close the quiz
                            answer4.visible = false

                            // Redraw
                            if (mapView)
                            {
                                mapView.redraw()
                            }
                        }
                    }
                }
            }
        ]
    }

    // Please try again
    CustomPopup {
        id: tryAgainDialog

        text: qsTr("The answer is not correct. Please try again.");

        buttons: [
            TextButton {
                text: qsTr("Ok")
                onClicked: {
                    pageTravailloscope.resetCheckBoxes(checkBoxRepeater1)
                    question1Column.selectedCount = 0
                    pageTravailloscope.resetCheckBoxes(checkBoxRepeater2)
                    question2Column.selectedCount = 0
                    tryAgainDialog.close()
                }
            }
        ]
    }

    // Confirm Reset Spot Dialog
    CustomPopup {
        id: confirmResetSpotDialog

        title: qsTr("Confirm action")
        text: qsTr("Are you sure to restart this finished spot?");
        text2: qsTr("All your answers will be cleared.")

        buttons: [
            Components.NoButton {
                text: qsTr("No")
                onClicked: {
                    confirmResetSpotDialog.close();
                }
            },

            Components.YesButton {
                text: qsTr("Yes")
                onClicked: {
                    confirmResetSpotDialog.close();
                    pageTravailloscope.startQuiz();
                }
            }
        ]
    }

    // Introduction screen
    DialogWindow {
        id: introductionScreen
        dialogHeader.text: ""
        visible: false

        content: [
            Column {
                width: parent.width
                height: childrenRect.height

                // Text
                Text {
                    id: introductionText
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
                        text: qsTr("Close")
                        onClicked: {
                            introductionScreen.visible = false;
                            Backend.setIntroductionSiteVisible(false)
                        }
                    }
                }
            }
        ]
    }
}
