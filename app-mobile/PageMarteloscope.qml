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
import QtCore
import ForDil
import "." 1.0

Rectangle {
    id: pageMarteloscope
    objectName: "PageMarteloscope"

    property int contentMargins: 0

    Component.onCompleted: {
        Globals.pageMarteloscope = pageMarteloscope
        console.debug("PageMarteloscope completed");

        if (Backend.isIntroductionSiteVisible())
        {
            introductionScreen.dialogHeader.text = Backend.introductionSiteHeader()
            introductionText.text = Backend.introductionSiteBody()
            introductionScreen.visible = true
        }
    }

    function isBlockedByOverlay() {
        return introductionScreen.visible || dropdownExplanation.visible
    }

    // Tree selection map
    property alias mapView: marteloscopeMapView

    MapView {
        id: marteloscopeMapView
        enabled: !pageMarteloscope.isBlockedByOverlay()
        anchors.fill: parent
        mapImageId: "mapImage"
    }

    function showTreeThinningMenu() {
        dropdown.visible = true
    }

    function hideTreeThinningMenu() {
        dropdown.visible = false
    }

    // Tree selection choices: explanation text
    property string competitionExplanationText: Backend.text("Marteloscope: Competition Explanation")
    property string maturityExplanationText: Backend.text("Marteloscope: Maturity Explanation")
    property string promoteRegenerationExplanationText: Backend.text("Marteloscope: Promote Regeneration Explanation")
    property string promoteRareSpeciesExplanationText: Backend.text("Marteloscope: Promote Rare Species Explanation")
    property string sanitaryExplanationText: Backend.text("Marteloscope: Sanitary Explanation")
    property string technicalExplanationText: Backend.text("Marteloscope: Technical Explanation")
    property string badQualityExplanationText: Backend.text("Marteloscope: Bad Quality Explanation")

    // Tree selection choices
    property string competitionText: Backend.text("Marteloscope: Competition Header")
    property string maturityText: Backend.text("Marteloscope: Maturity Header")
    property string promoteRegenerationText: Backend.text("Marteloscope: Promote Regeneration Header")
    property string promoteRareSpeciesText: Backend.text("Marteloscope: Promote Rare Species Header")
    property string sanitaryText: Backend.text("Marteloscope: Sanitary Header")
    property string technicalText: Backend.text("Marteloscope: Technical Header")
    property string badQualityText: Backend.text("Marteloscope: Bad Quality Header")

    // Selection choice (from 1 to 6)
    property int selectionChoice: 1

    property string selectedText: {
        switch (selectionChoice) {
        case 1: return competitionText;
        case 2: return maturityText;
        case 3: return promoteRegenerationText;
        case 4: return promoteRareSpeciesText;
        case 5: return sanitaryText;
        case 6: return technicalText;
        case 7: return badQualityText;
        default: return "N/A";
        }
    }

    property string selectedExplanationText: {
        switch (selectionChoice) {
        case 1: return competitionExplanationText;
        case 2: return maturityExplanationText;
        case 3: return promoteRegenerationExplanationText;
        case 4: return promoteRareSpeciesExplanationText;
        case 5: return sanitaryExplanationText;
        case 6: return technicalExplanationText;
        case 7: return badQualityExplanationText;
        default: return "N/A";
        }
    }

    // Toggle
    function initToggle() {
        toggleC1.checked = Backend.isCompetitionChecked();
        toggleC2.checked = Backend.isMaturityChecked();
        toggleC3.checked = Backend.isPromoteRegenerationChecked();
        toggleC4.checked = Backend.isPromoteRareSpeciesChecked();
        toggleC5.checked = Backend.isSanitaryChecked();
        toggleC6.checked = Backend.isTechnicalChecked();
        // toggleC7.checked = Backend.isBadQualityChecked();

        treeId.text = Backend.selectedTreeId();
        treeSpecies.text = Backend.selectedTreeSpecies();
        treeDBH.text = Backend.selectedTreeDBH();
    }

    function resetToggle() {
        toggleC1.checked = false;
        toggleC2.checked = false;
        toggleC3.checked = false;
        toggleC4.checked = false;
        toggleC5.checked = false;
        toggleC6.checked = false;
        // toggleC7.checked = false;

        treeId.text = "0"
        treeSpecies.text = "N/A"
        treeDBH.text = "0"

        // Redraw
        if (mapView)
        {
            mapView.redraw()
        }
    }

    // Information about selected tree
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10

        color: Qt.rgba(1, 1, 1, 0.85)

        implicitWidth: treeInformationGrid.implicitWidth + 16
        implicitHeight: treeInformationGrid.implicitHeight + 16

        GridLayout {
            id: treeInformationGrid
            columns: 2
            rowSpacing: 6
            columnSpacing: 12
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 8

            // first column right-aligned so all "|" line up
            Text { id: treeId;       text: "0";    Layout.alignment: Qt.AlignRight }
            Text { text: qsTr("Tree ID") }

            Text { id: treeDBH;      text: "0";    Layout.alignment: Qt.AlignRight }
            Text { text: qsTr("DBH [cm]") }

            Text { id: treeSpecies;  text: "N/A";  Layout.alignment: Qt.AlignRight }
            Text { text: qsTr("Species") }
        }
    }

    // End exercise button
    Components.EndButton {
        text: qsTr("End")
        enabled: !pageMarteloscope.isBlockedByOverlay()
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
                    Globals.pageMain.pushPage("PageGlobalResults.qml")
                }
            }
        ]
    }

    // Dropdown tree thinning reason menu
    Rectangle {
        id: dropdown

        width: 210 * Globals.bigScreenMultiplier
        height: 400 * Globals.bigScreenMultiplier

        color: "#ffffff"
        border.color: "gray"
        enabled: !pageMarteloscope.isBlockedByOverlay()
        visible: false

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: pageMarteloscope.contentMargins

        readonly property int menuItemWidth: 170 * Globals.bigScreenMultiplier
        readonly property int menuItemHeight: 50 * Globals.bigScreenMultiplier
        readonly property int menuItemToggleWidth: 100 * Globals.bigScreenMultiplier
        readonly property int menuItemToggleHeight: 35 * Globals.bigScreenMultiplier
        readonly property int menuItemFontSize: 10 * Globals.bigScreenMultiplier
        readonly property int menuIconDim: 25 * Globals.bigScreenMultiplier
        readonly property int menuIconHelpDim: 30 * Globals.bigScreenMultiplier
        readonly property int menuIconHelpMargin: 2 * Globals.bigScreenMultiplier

        Column {
            id: dropdownColumn
            anchors.fill: parent
            anchors.margins: 5
            spacing: 5 * Globals.bigScreenMultiplier

            // Competition
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    height: dropdown.menuItemHeight
                    ToolButton {
                        id: toggleC1
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_1.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.competitionText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC1.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 1
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Maturity
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    height: dropdown.menuItemHeight
                    ToolButton {
                        id: toggleC2
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_2.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.maturityText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC2.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 2
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Promote regeneration
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    height: dropdown.menuItemHeight
                    ToolButton {
                        id: toggleC3
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_3.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.promoteRegenerationText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC3.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 3
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Promote rare species
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    height: dropdown.menuItemHeight
                    ToolButton {
                        id: toggleC4
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_4.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.promoteRareSpeciesText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC4.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 4
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Sanitary
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    height: dropdown.menuItemHeight
                    ToolButton {
                        id: toggleC5
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_5.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.sanitaryText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC5.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 5
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Technical
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    ToolButton {
                        id: toggleC6
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_6.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.technicalText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC6.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 6
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }

            // Bad Quality
            /*
            Rectangle {
                width: dropdown.menuItemWidth
                height: dropdown.menuItemHeight
                Row {
                    ToolButton {
                        id: toggleC7
                        width: dropdown.menuItemWidth
                        height: dropdown.menuItemHeight
                        checkable: true
                        checked: false

                        contentItem: Column {
                            spacing: 4
                            anchors.centerIn: parent

                            Image {
                                source: "qrc:/images/C_7.png"
                                width: dropdown.menuIconDim
                                height: dropdown.menuIconDim
                                fillMode: Image.PreserveAspectFit
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: pageMarteloscope.badQualityText
                                font.pointSize: dropdown.menuItemFontSize
                                horizontalAlignment: Text.AlignHCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "black"
                            }
                        }

                        background: Rectangle {
                            implicitWidth: dropdown.menuItemToggleWidth
                            implicitHeight: dropdown.menuItemToggleHeight
                            color: toggleC7.checked ? "lightgray" : "transparent"
                        }
                    }

                    // Spacer
                    Rectangle {
                        width: dropdown.menuIconHelpMargin
                        height: 1
                        color: "transparent"
                    }

                    // Help
                    Image {
                        source: "qrc:/images/ui_question.png"

                        fillMode: Image.PreserveAspectFit
                        width: dropdown.menuIconHelpDim
                        height: dropdown.menuIconHelpDim
                        anchors.leftMargin: 5

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageMarteloscope.selectionChoice = 7
                                dropdownExplanation.dialogHeader.text = pageMarteloscope.selectedText
                                explanationText.text = pageMarteloscope.selectedExplanationText
                                dropdownExplanation.visible = true
                            }
                        }
                    }
                }
            }
            */

            // Spacer
            Item {
                height: 1
                width: 1
            }

            // Cancel & Confirm selection buttons
            Rectangle {
                id: dropdownActions

                readonly property int menuWidth: 180 * Globals.bigScreenMultiplier
                readonly property int menuHeight: 40 * Globals.bigScreenMultiplier
                readonly property int menuButtonWidth: 80 * Globals.bigScreenMultiplier
                readonly property int menuButtonHeight: 40 * Globals.bigScreenMultiplier

                width: dropdownActions.menuWidth 
                height: dropdownActions.menuHeight

                anchors.horizontalCenter: parent.horizontalCenter

                Row {
                    width: dropdownActions.menuWidth
                    height: dropdownActions.menuHeight

                    Button {
                        text: qsTr("Cancel")
                        font.pointSize: dropdown.menuItemFontSize
                        width: dropdownActions.menuButtonWidth
                        height: dropdownActions.menuButtonHeight

                        onClicked: {
                            pageMarteloscope.hideTreeThinningMenu()
                            Backend.cancelSelection()
                            pageMarteloscope.resetToggle()
                        }

                        background: Rectangle {
                            color: "#FF6666"
                            border.color: "#fd4646"
                            radius: 7
                            border.width: 2
                        }
                    }

                    Item {
                        width: 10 * Globals.bigScreenMultiplier
                        height: 1
                    }

                    Button {
                        text: qsTr("Confirm")
                        font.pointSize: dropdown.menuItemFontSize
                        width: dropdownActions.menuButtonWidth
                        height: dropdownActions.menuButtonHeight

                        onClicked: {
                            pageMarteloscope.hideTreeThinningMenu()
                            Backend.confirmSelection(toggleC1.checked,
                                                     toggleC2.checked,
                                                     toggleC3.checked,
                                                     toggleC4.checked,
                                                     toggleC5.checked,
                                                     toggleC6.checked,
                                                     false) // toggleC7.checked)
                            pageMarteloscope.resetToggle()
                        }

                        background: Rectangle {
                            color: "#41B28E"
                            border.color: "#26787A"
                            // color: "lightgreen"
                            radius: 7
                            border.width: 2
                        }
                    }
                }
            }

        }
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

    // Dropdown explanation screen
    DialogWindow {
        id: dropdownExplanation
        dialogHeader.text: ""
        hasClose: true
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
                    height: 10
                    color: "transparent"
                }
            }
        ]
    }

    // Close button
    CloseButton {
        id: closeButton
        visible: dropdownExplanation.visible

        MouseArea {
            anchors.fill: parent
            onClicked: {
                dropdownExplanation.visible = false
            }
        }
    }
}
