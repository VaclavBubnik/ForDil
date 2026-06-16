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
    id: pageGlobalResults

    property int chapterSpacing: 20

    QtObject {
        id: helpers
        function componentWidth() { return 350 }
    }

    property var _init: (function() {
        Backend.calculateTypologyLocal();
        Backend.calculateTypologyGlobal();
        Backend.calculateOverallAssessment();
        return 0
    })()

    Component.onCompleted: {
        Globals.pageGlobalResults = pageGlobalResults
        console.debug("PageGlobalResults completed");

        if (Backend.isIntroductionGlobalResultsVisible())
        {
            pedagogicalContents.dialogHeader.text = Backend.text("Results: Global Results Introduction Header")
            pedagogicalContentsText.text = Backend.text("Results: Global Results Introduction")
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
    component OverallAssessment: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_01.png"
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

    // 2
    component ResultTreeStatistics: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_02.png"
            headerText: "Results: Tree Statistics Header"
            explanationHeader: "Results: Tree Statistics Explanation Header"
            explanationKey: "Results: Tree Statistics Explanation"

            Switch {
                id: showSpeciesSwitch
                text: qsTr("Show species")
                width: implicitWidth
                height: 30

                contentItem: Text {
                    text: showSpeciesSwitch.text
                    color: "#26787A"
                    font: showSpeciesSwitch.font
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: showSpeciesSwitch.indicator.width + showSpeciesSwitch.spacing
                }

                checked: Backend.isSpeciesInTreeStatisticsEnabled()
                onCheckedChanged: {
                    Backend.setSpeciesInTreeStatisticsEnabled(checked); // Update settings
                    treeStatisticsPlot.height = Backend.imageHeight("treeStatisticsImage", treeStatisticsPlot.width);
                    treeStatisticsImage.redraw();
                }
            }
        }

        Rectangle {
            id: treeStatisticsPlot
            width: parent.width
            height: { return Backend.imageHeight("treeStatisticsImage", parent.width); }

            Image {
                id: treeStatisticsImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: treeStatisticsImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/treeStatisticsImage"

                function redraw() {
                    treeStatisticsImage.source = ""
                    treeStatisticsImage.source = treeStatisticsImage.imageSource
                }

                Component.onCompleted: {
                    treeStatisticsPlot.height = Backend.imageHeight("treeStatisticsImage", treeStatisticsPlot.width);
                }
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 3
    component ResultEconomicUserValues: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_03.png"
            headerText: "Results: Economic Values Header"
            explanationHeader: "Results: Economic Values Explanation Header"
            explanationKey: "Results: Economic Values Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 4

            Image {
                id: economicUserValuesImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: economicUserValuesImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/economicUserValuesImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 4
    component ResultEconomicExpertValues: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_04.png"
            headerText: "Results: Economic Values (Expert) Header"
            explanationHeader: "Results: Economic Values Explanation Header"
            explanationKey: "Results: Economic Values Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 4

            Image {
                id: economicExpertValuesImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: economicExpertValuesImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/economicExpertValuesImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 5
    component ResultTreeDistribution: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_05.png"
            headerText: "Results: Tree Distribution Header"
            explanationHeader: "Results: Tree Distribution Explanation Header"
            explanationKey: "Results: Tree Distribution Explanation"

            Item {
                width: treeDistributionGhaNhaSwitch.width
                height: 30

                // Settings
                Row {
                    id: treeDistributionGhaNhaSwitch
                    height: parent.height
                    spacing: 6

                    Label {
                        id: showTreeDistributionGhaLabel
                        text: qsTr("GHA")
                        color: "#26787A"
                        width: implicitWidth
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Switch {
                        id: showTreeDistributionGhaNhaSwitch
                        text: qsTr("NHA")
                        width: implicitWidth
                        anchors.verticalCenter: parent.verticalCenter

                        contentItem: Text {
                            text: showTreeDistributionGhaNhaSwitch.text
                            color: "#26787A"
                            font: showTreeDistributionGhaNhaSwitch.font
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: showTreeDistributionGhaNhaSwitch.indicator.width + showTreeDistributionGhaNhaSwitch.spacing
                        }

                        Component.onCompleted: {
                            checked = Backend.treeDistributionOption === "NHA"
                        }
                        onCheckedChanged: {
                            let mode = checked ? "NHA" : "GHA";
                            Backend.setTreeDistributionOption(mode); // Update settings
                            treeDistributionImage.redraw(); // Refresh data
                        }
                    }
                }
            }
        }

        Item {
            visible: Backend.treeDistributionOption === "NHA"
            width: parent.width
            height: 5
        }

        Item {
            id: curveDropDown
            width: parent.width
            height: 50

            visible: Backend.treeDistributionOption === "NHA"

            property var selectedItems: []

            // Define the model with ID so you can modify it
            ListModel {
                id: curveOptionsModel
            }

            RowLayout {
                // Dropdown button
                Button {
                    id: dropDownButton
                    text: qsTr("Show CCF Model Curves")
                    // anchors.top: parent.top
                    // anchors.left: parent.left

                    onClicked: {
                        // Load options when opening popup
                        curveOptionsModel.clear()
                        let options = Backend.populateCurveOptions()
                        for (let i = 0; i < options.length; ++i) {
                            curveOptionsModel.append(options[i])
                        }
                        dropDown.open()
                    }
                }

                // Explanation
                Item {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30

                    Image {
                        anchors.fill: parent
                        source: "qrc:/images/ui_question.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            bus.showPedagogical("Results: CCF Model Curves Explanation",
                                                "Results: CCF Model Curves Explanation Header")
                            // pageGlobalResults.pedagogicalContents.visible = true
                            // pedagogicalContents.dialogHeader.text = Backend.text("Results: CCF Model Curves Explanation Header")
                            // pedagogicalContentsText.text = Backend.text("Results: CCF Model Curves Explanation")
                        }
                    }
                }
            }

            Popup {
                id: dropDown
                width: parent.width
                y: dropDownButton.height
                modal: true
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                background: null  // Disable any default background
                padding: 0

                onVisibleChanged: {
                    if (!visible) {
                        // Popup was just closed, send data to C++
                        Backend.setSelectedCurveOptions(curveDropDown.selectedItems);
                        treeDistributionImage.redraw();
                    }
                }

                Rectangle {
                    width: parent.width
                    height: Math.min(curveOptionsModel.count * 40, 400) // 400 = max height if needed
                    color: "#D5EFE0"

                    border.color: "#41B28E" // Optional: subtle border
                    radius: 4                 // Optional: rounded corners

                    ListView {
                        anchors.fill: parent

                        model: curveOptionsModel

                        delegate: Item {
                            width: ListView.view.width
                            height: 40

                            CheckBox {
                                id: checkbox
                                anchors.verticalCenter: parent.verticalCenter
                                text: model.text
                                checked: false

                                Component.onCompleted: {
                                    checked = model.checked
                                }

                                onCheckedChanged: {
                                    model.checked = checked
                                    if (checked) {
                                        if (!curveDropDown.selectedItems.includes(model.id)) {
                                            curveDropDown.selectedItems.push(model.id)
                                        }
                                    } else {
                                        curveDropDown.selectedItems = curveDropDown.selectedItems.filter(i => i !== model.id)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } // curve options

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: treeDistributionImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: treeDistributionImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/treeDistributionImage"

                function redraw() {
                    treeDistributionImage.source = ""
                    treeDistributionImage.source = treeDistributionImage.imageSource
                }
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 6
    component ResultGiniIndex: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_06.png"
            headerText: "Results: Gini Index Header"
            explanationHeader: "Results: Gini Index Explanation Header"
            explanationKey: "Results: Gini Index Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 3

            Image {
                id: giniIndexImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: giniIndexImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/giniIndexImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 7
    component ResultThinningFactor: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_07.png"
            headerText: "Results: K Factor Header"
            explanationHeader: "Results: K Factor Explanation Header"
            explanationKey: "Results: K Factor Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 1

            Image {
                id: thinningFactorImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: thinningFactorImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/thinningFactorImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 8
    component ResultQualityBefore: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_08.png"
            headerText: "Results: Quality - before Header"
            explanationHeader: "Results: Quality Explanation Header"
            explanationKey: "Results: Quality Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: qualityBeforeImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: qualityBeforeImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/qualityBeforeImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    component ResultQualityCut: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_08.png"
            headerText: "Results: Quality - cut Header"
            explanationHeader: "Results: Quality Explanation Header"
            explanationKey: "Results: Quality Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: qualityCutImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: qualityCutImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/qualityCutImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    component ResultQualityAfter: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_08.png"
            headerText: "Results: Quality - after Header"
            explanationHeader: "Results: Quality Explanation Header"
            explanationKey: "Results: Quality Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: qualityAfterImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: qualityAfterImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/qualityAfterImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 9
    component ResultQualityEvolution: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_09.png"
            headerText: "Results: Quality Evolution Header"
            explanationHeader: "Results: Quality Explanation Header"
            explanationKey: "Results: Quality Explanation"
        }

        Rectangle {
            width: 200
            height: 100

            Image {
                id: qualityEvolutionImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: qualityEvolutionImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/qualityEvolutionImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 10
    component ResultCompositionBefore: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_10.png"
            headerText: "Results: Composition - before Header"
            explanationHeader: "Results: Composition Explanation Header"
            explanationKey: "Results: Composition Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: compositionBeforeImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: compositionBeforeImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/compositionBeforeImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    component ResultCompositionCut: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_10.png"
            headerText: "Results: Composition - cut Header"
            explanationHeader: "Results: Composition Explanation Header"
            explanationKey: "Results: Composition Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: compositionCutImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: compositionCutImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/compositionCutImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    component ResultCompositionAfter: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_10.png"
            headerText: "Results: Composition - after Header"
            explanationHeader: "Results: Composition Explanation Header"
            explanationKey: "Results: Composition Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: compositionAfterImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: compositionAfterImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/compositionAfterImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 11
    component ResultShannonIndex: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_11.png"
            headerText: "Results: Shannon Index Header"
            explanationHeader: "Results: Shannon Index Explanation Header"
            explanationKey: "Results: Shannon Index Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 3

            Image {
                id: shannonIndexImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: shannonIndexImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/shannonIndexImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 12
    component ResultChoicesAssessment: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_12.png"
            headerText: "Results: Choices Assessment Header"
            explanationHeader: "Results: Choices Assessment Explanation Header"
            explanationKey: "Results: Choices Assessment Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: choicesAssessmentImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: choicesAssessmentImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/choicesAssessmentImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 13
    component ResultEcologicalValuesDeadBio: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_13.png"
            headerText: "Results: Dead and Biological Trees Header"
            explanationHeader: "Results: Dead and Biological Trees Explanation Header"
            explanationKey: "Results: Dead and Biological Trees Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 4

            Image {
                id: ecologicalValuesDeadBioImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: ecologicalValuesDeadBioImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/ecologicalValuesDeadBioImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 14
    component ResultEcologicalValues: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_14.png"
            headerText: "Results: Prosilva Header"
            explanationHeader: "Results: Prosilva Explanation Header"
            explanationKey: "Results: Prosilva Explanation"
        }

        Rectangle {
            width: parent.width
            height: width * 0.1 * 4

            Image {
                id: ecologicalValuesImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: ecologicalValuesImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/ecologicalValuesImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }

    // 15
    component ResultEcologicalValuesDmh: Column {
        width: Globals.resultComponentAutoWidth()

        ChapterHeader {
            chapterImage: "qrc:/images/ui_number_15.png"
            headerText: "Results: Dendromicrohabitats (DMH) Header"
            explanationHeader: "Results: Dendromicrohabitats (DMH) Explanation Header"
            explanationKey: "Results: Dendromicrohabitats (DMH) Explanation"
        }

        Rectangle {
            width: parent.width
            height: parent.width

            Image {
                id: ecologicalValuesDmhImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: ecologicalValuesDmhImage.imageSource
                sourceSize: Qt.size(width, height)
                cache: false

                readonly property string imageSource: "image://fordilimageprovider/ecologicalValuesDmhImage"
            }
        }

        Item {
            width: parent.width
            height: 20
        }
    }
    // -------------------------------------------------------------------------

    // Results
    ZoomableView {
        enabled: !pageGlobalResults.isBlockedByOverlay()

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
                    OverallAssessment { id: overallAssessment }
                    ResultTreeStatistics { id: resultTreeStatistics }
                    ResultEconomicUserValues { id: resultEconomicUserValues }
                    ResultEconomicExpertValues { id: resultEconomicExpertValues }
                }

                Flow {
                    width: contentColumn.width
                    ResultTreeDistribution { id: resultTreeDistribution }
                    ResultGiniIndex { id: resultGiniIndex }
                    ResultThinningFactor { id: resultThinningFactor }
                }

                Flow {
                    width: contentColumn.width
                    ResultQualityBefore { id: resultQualityBefore }
                    ResultQualityCut { id: resultQualityCut }
                    ResultQualityAfter { id: resultQualityAfter }
                    ResultQualityEvolution { id: resultQualityEvolution }
                }

                Flow {
                    width: contentColumn.width
                    ResultCompositionBefore { id: resultCompositionBefore }
                    ResultCompositionCut { id: resultCompositionCut }
                    ResultCompositionAfter { id: resultCompositionAfter }
                    ResultShannonIndex { id: resultShannonIndex }
                }

                ResultChoicesAssessment { id: resultChoicesAssessment }

                Flow {
                    width: contentColumn.width
                    ResultEcologicalValuesDeadBio { id: resultEcologicalValuesDeadBio }
                    ResultEcologicalValues { id: resultEcologicalValues }
                    ResultEcologicalValuesDmh { id: resultEcologicalValuesDmh }
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
                Backend.setIntroductionGlobalResultsVisible(false)
            }
        }
    }
}
