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
    id: mapView

    property string mapImageId: "default"
    readonly property string mapImageSource: "image://fordilimageprovider/" + mapImageId
    property int contentMargins: 0

    property string previousLocation: ""

    // property alias gpsVisible: gpsSettings.visible

    // Custom method to force a redraw or refresh
    function redraw(external = false) {
        if (!external || Backend.isRedrawAllowed())
        {
            mapImage.source = "";
            mapImage.source = mapView.mapImageSource;
        }
    }

    // Updates the current location
    function toDMS(decimal, isLatitude) {
        var deg = Math.floor(Math.abs(decimal));
        var minFloat = (Math.abs(decimal) - deg) * 60;
        var min = Math.floor(minFloat);
        var sec = ((minFloat - min) * 60).toFixed(2);

        var direction;
        if (isLatitude)
            direction = decimal >= 0 ? "N" : "S";
        else
            direction = decimal >= 0 ? "E" : "W";

        return deg + "° " + min + "' " + sec + "\" " + direction;
    }

    function updateLocation(latitude, longitude) {
        var wasLocationAvailable = Backend.isLocationAvailable
        if (wasLocationAvailable === false) {
            Backend.setLocationAvailable(true);
            previousLocation = ""
        }

        const newLocation = `Lat: ${latitude.toFixed(7)}, Lon: ${longitude.toFixed(7)}`;
        if (newLocation !== previousLocation) {
            previousLocation = newLocation;

            gpsEnabledStateText.text = qsTr("On")

            gpsLatitude.visible = Backend.isShowGpsCoordinatesEnabled;
            gpsLatitudeText.text = toDMS(latitude, true);
            gpsLongitude.visible = Backend.isShowGpsCoordinatesEnabled;
            gpsLongitudeText.text = toDMS(longitude, false);

            Backend.setLocation(latitude, longitude);
            redraw(true);
        }
    }

    // Updates the current azimuth
    property real previousAzimuth: 0
    property real minAzimuthDelta: 1.0
    property real mapAzimuth: 0

    // 0 < alpha <= 1; smaller = more smoothing
    readonly property real alpha: 0.1
    property real filteredAzimuth: 0
    property bool hasInit: false

    function updateAzimuth(rawAzimuth) {
        // 1) Initialize on first reading
        if (!hasInit) {
            filteredAzimuth = rawAzimuth
            previousAzimuth = rawAzimuth
            mapAzimuth = -rawAzimuth
            hasInit = true
            return
        }

        // 2) Low-pass filter (circular)
        var diff = rawAzimuth - filteredAzimuth
        if (diff > 180)
            diff -= 360
        else if (diff < -180)
            diff += 360
        
        filteredAzimuth = filteredAzimuth + alpha * diff

        // Normalize 0–360
        if (filteredAzimuth < 0)
            filteredAzimuth += 360
        else if (filteredAzimuth >= 360)
            filteredAzimuth -= 360

        // 3) Dead zone on filtered value before updating UI
        var displayDiff = filteredAzimuth - previousAzimuth
        if (displayDiff > 180)
            displayDiff -= 360
        else if (displayDiff < -180)
            displayDiff += 360

        if (Math.abs(displayDiff) > minAzimuthDelta) {
            previousAzimuth = filteredAzimuth
            mapAzimuth = -filteredAzimuth
            Backend.setAzimuth(filteredAzimuth)
            Backend.setAzimuthAvailable(true)
            redraw(true);
        }
    }

    function showSelectedPointInformation() {
        const latitude = Backend.selectedPointLatitude();
        const longitude = Backend.selectedPointLongitude();
        const pointLocation = `Lat: ${latitude.toFixed(6)}, Lon: ${longitude.toFixed(6)}`;
        selectedPointText.text = pointLocation;
        selectedPointInformation.visible = Backend.isDebugModeEnabled;
    }

    // The map area
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        property real scaleFactor: 1.0

        // The 'source' property uses ForDil image provider
        Image {
            id: mapImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: mapView.mapImageSource
            sourceSize: Qt.size(width, height)
            cache: false
        }

        // PinchArea handles user touch interactions:
        // - Multi-touch gestures like pinch-to-zoom
        // - Single-touch events are passed to child items like MouseArea (for taps/clicks)
        PinchArea {
            id: pinchArea
            anchors.fill: parent

            // Initial and current zoom factors
            property real initialZoom: 1.0
            property real currentZoom: 1.0
            property real initialX: 0
            property real initialY: 0

            onPinchStarted: (pinch) => {
                initialZoom = Backend.zoom(mapImageId)
                initialX = pinch.center.x
                initialY = pinch.center.y
            }

            onPinchUpdated: (pinch) => {
                currentZoom = initialZoom * pinch.scale

                Backend.setZoom(mapImageId,
                                currentZoom,
                                pinch.scale,
                                pinch.center.x,
                                pinch.center.y,
                                pinchArea.width,
                                pinchArea.height);

                mapView.redraw()
            }

            onPinchFinished: (pinch) => {
                // No-op: currentZoom already updated
            }

            // MouseArea handles user tap or click interactions (single-point)
            MouseArea {
                id: panArea
                anchors.fill: parent
                property real firstX
                property real firstY
                property real lastX
                property real lastY
                property real totalChangeX
                property real totalChangeY
                property real dragThreshold: 10  // Minimum movement to detect drag

                // Drag start
                onPressed: (mouse) => {
                    firstX = mouse.x;
                    firstY = mouse.y;
                    lastX = firstX;
                    lastY = firstY;
                }

                // Drag
                onPositionChanged: (mouse) => {
                    var dx = mouse.x - lastX;
                    var dy = mouse.y - lastY;

                    lastX = mouse.x;
                    lastY = mouse.y;

                    Backend.translate(mapImageId, dx, dy);

                    // Redraw
                    mapView.redraw()
                }

                onReleased: (mouse) => {
                    lastX = mouse.x;
                    lastY = mouse.y;

                    totalChangeX = Math.abs(lastX - firstX);
                    totalChangeY = Math.abs(lastY - firstY);

                    if (totalChangeX < dragThreshold && totalChangeY < dragThreshold)
                    {
                        if (Qt.inputMethod.visible) {
                            return
                        }

                        if (Backend.loadedSystem() === "Marteloscope")
                        {
                            if (Backend.loadedDiscoveryPart())
                            {
                                // Discovery
                                if (Backend.click(mapImageId, mouse.x, mouse.y, panArea.width, panArea.height))
                                {
                                    explanation.dialogHeader.text = Backend.discoveryHeader();
                                    explanationText.text = Backend.discoveryText();
                                    explanation.visible = true;
                                }
                            }
                            else
                            {
                                // Click in Tree selection
                                if (!Backend.isExerciseFinished && !dropdown.visible)
                                {
                                    if (Backend.click(mapImageId, mouse.x, mouse.y, panArea.width, panArea.height))
                                    {
                                        Backend.translateToSelection(mapImageId)
                                        pageMarteloscope.initToggle()
                                        pageMarteloscope.showTreeThinningMenu()
                                    }

                                    mapView.showSelectedPointInformation()
                                }

                                // Click in Hotspots
                                if (Backend.isExerciseFinished && Backend.hotspotsReasonText() !== "None")
                                {
                                    if (Backend.click(mapImageId, mouse.x, mouse.y, panArea.width, panArea.height))
                                    {
                                        pedagogicalContents.dialogHeader.text = Backend.hotspotsPedagogicalHeader()
                                        pedagogicalContentsText.text = Backend.hotspotsPedagogicalContents()
                                        pedagogicalContents.visible = true
                                    }

                                    mapView.showSelectedPointInformation()
                                }
                            }
                        }
                        else
                        {
                            // Click in Travailoscope
                            if (!Backend.isExerciseFinished)
                            {
                                if (Backend.click(mapImageId, mouse.x, mouse.y, panArea.width, panArea.height))
                                {
                                    if (Backend.isTravailloscopeSpotFinished())
                                    {
                                        confirmResetSpotDialog.open();
                                    }
                                    else
                                    {
                                        pageTravailloscope.startQuiz();
                                    }
                                }

                                mapView.showSelectedPointInformation()
                            }

                            // Click in Travailoscope valuation
                            if (Backend.isExerciseFinished && Backend.valuationReasonText() !== "None")
                            {
                                if (Backend.click(mapImageId, mouse.x, mouse.y, panArea.width, panArea.height))
                                {
                                    pedagogicalContents.dialogHeader.text = Backend.valuationPedagogicalHeader()
                                    pedagogicalContentsText.text = Backend.valuationPedagogicalContents()
                                    pedagogicalContents.visible = true
                                }

                                mapView.showSelectedPointInformation()
                            }
                        }

                        // Redraw
                        mapView.redraw()
                    }
                }
            }
        }
    }

    // Top bar
    RowLayout {
        id: topRow
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 0
        spacing: 0  // We'll use spacers instead

        // Fixed-width spacer at the front
        Rectangle {
            id: spacerLeft
            Layout.preferredWidth: 1
            Layout.preferredHeight: 30
            color: "transparent"
        }

        // Search
        Row {
            id: searchBar
            spacing: 8

            function translateToId(idText) {
                if (idText === "")
                {
                    return
                }

                const found = Backend.translateToId(idText)

                if (!found)
                {
                    if (!Backend.loadedDiscoveryPart())
                    {
                        if (Backend.loadedSystem() === "Marteloscope")
                        {
                            treeId.text = "0"
                            treeSpecies.text = "N/A"
                            treeDBH.text = "0"
                        }
                    }

                    searchField.text = ""   // Clears the TextField
                    idNotFoundDialog.open() // Open error dialog
                }
                else
                {
                    if (!Backend.loadedDiscoveryPart())
                    {
                        if (Backend.loadedSystem() === "Marteloscope")
                        {
                            treeId.text = Backend.selectedTreeId();
                            treeSpecies.text = Backend.selectedTreeSpecies();
                            treeDBH.text = Backend.selectedTreeDBH();
                        }
                    }
                }

                mapView.redraw()
            }

            Item {
                id: searchMagnifierIcon
                width: 40
                height: 40

                Image {
                    anchors.fill: parent
                    source: "qrc:/images/ui_magnifier.svg"
                    fillMode: Image.PreserveAspectFit
                }

                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -10
                    onClicked: searchBar.translateToId(searchField.text);

                    // Debug
                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "red"
                    //     opacity: 0.2
                    // }
                }
            }

            Item {
                id: searchBox
                width: measuredText.width + 16
                height: 40
                anchors.verticalCenter: searchMagnifierIcon.verticalCenter

                // Invisible text used to measure the placeholder
                Text {
                    id: measuredText
                    text: {
                        if (Backend.loadedSystem() === "Marteloscope")
                        {
                            return qsTr("Tree n°")
                        }
                        else
                        {
                            return qsTr("Spot n°")
                        }
                    }

                    font: searchField.font // Match font for accurate size
                    visible: false
                }

                // Put this somewhere accessible (e.g., inside same parent as searchField)
                Item {
                    id: focusSink
                    width: 0
                    height: 0
                    focus: true // can take focus
                }

                TextField {
                    id: searchField
                    anchors.fill: parent
                    placeholderText: measuredText.text
                    inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText

                    // integer validator
                    validator: IntValidator { bottom: 0; top: 999999 }

                    onAccepted: {
                        Qt.inputMethod.hide()
                        focusSink.forceActiveFocus()
                    }

                    Keys.onPressed: (event) => {
                        if (event.key === 16777217 || event.key === 16777220) {
                            event.accepted = true
                            Qt.inputMethod.hide()
                            focusSink.forceActiveFocus()

                            searchBar.translateToId(text);
                        }
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Elapsed time
        Rectangle {
            id: elapsedTimeInformation

            visible: true

            Layout.preferredWidth: elapsedTimeLabel.paintedWidth + 10
            Layout.preferredHeight: elapsedTimeLabel.paintedHeight + 10

            color: Qt.rgba(1, 1, 1, 0.85)

            property string elapsedTime: Backend.elapsedExerciseTime()

            Timer {
                interval: 1000 // update every second
                running: true
                repeat: true
                onTriggered: {
                    elapsedTimeInformation.elapsedTime = Backend.elapsedExerciseTime()
                }
            }

            Text {
                id: elapsedTimeLabel
                anchors.centerIn: parent
                text: elapsedTimeInformation.elapsedTime
            }
        }

        Item { Layout.fillWidth: true }

        // Compass
        Item {
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50

            Image {
                anchors.fill: parent
                source: "qrc:/images/ui_compass_arrow_n.png"
                fillMode: Image.PreserveAspectFit
                transform: Rotation {
                    origin.x: 25
                    origin.y: 25
                    angle: mapView.mapAzimuth
                }
            }
        }

        Item { Layout.fillWidth: true }

        // GPS Settings
        Rectangle {
            id: gpsSettings

            Layout.preferredWidth: gpsEnabledText.paintedWidth + gpsEnabledSwitch.width + 15
            Layout.preferredHeight: gpsEnabledSwitch.height + 10

            color: "transparent"

            Rectangle {
                anchors.fill: parent
                anchors.margins: 0

                Column {
                    Row {
                        spacing: 5

                        Label {
                            id: gpsEnabledText
                            text: qsTr("GPS")
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Rectangle {
                            id: gpsEnabledSwitch
                            width: 50
                            height: 24
                            radius: height / 2
                            color: checked ? "#000" : "#ccc"

                            property bool checked: false

                            signal toggled(bool checked)

                            Component.onCompleted: {
                                checked = Backend.isGpsEnabled
                            }

                            Rectangle {
                                id: thumb
                                width: 20
                                height: 20
                                radius: height / 2
                                anchors.verticalCenter: parent.verticalCenter
                                x: gpsEnabledSwitch.checked ? parent.width - width - 2 : 2
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    gpsEnabledSwitch.checked = !gpsEnabledSwitch.checked
                                    gpsEnabledSwitch.toggled(gpsEnabledSwitch.checked)
                                    Backend.setLocationAvailable(false);
                                    gpsLatitude.visible = false;
                                    gpsLongitude.visible = false;
                                    if (!gpsEnabledSwitch.checked)
                                    {
                                        gpsEnabledStateText.text = qsTr("Off")
                                    }
                                    else
                                    {
                                        gpsEnabledStateText.text = qsTr("No signal")
                                    }
                                    Backend.setGpsEnabled(gpsEnabledSwitch.checked); // Update settings
                                    mapView.redraw()
                                }
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    } // GPS Row

                    Rectangle {
                        id: gpsEnabledStateRectangle

                        width: gpsEnabledStateText.paintedWidth + 5
                        height: gpsEnabledStateText.paintedHeight + 2

                        color: Qt.rgba(1, 1, 1, 0.85)

                        Label {
                            id: gpsEnabledStateText;
                            anchors.centerIn: parent
                            text: "";
                            Component.onCompleted: {
                                text = gpsEnabledSwitch.checked ? qsTr("No signal") : qsTr("Off")
                            }
                        }
                    }
                }
            }
        }

        // Fixed-width spacer at the end
        Rectangle {
            id: spacer
            Layout.preferredWidth: 60
            Layout.preferredHeight: 30
            color: "transparent"
        }
    }

    // GPS Latitude
    Rectangle {
        id: gpsLatitude

        visible: false

        anchors.top: topRow.bottom
        anchors.right: parent.right
        anchors.margins: 0
        anchors.topMargin: 10

        width: gpsLatitudeText.paintedWidth + 20
        height: gpsLatitudeText.paintedHeight + 10

        color: Qt.rgba(1, 1, 1, 0.85)

        Text {
            id: gpsLatitudeText;
            anchors.centerIn: parent
            text: qsTr("No GPS");
        }
    }

    // GPS Longitude
    Rectangle {
        id: gpsLongitude

        visible: false

        anchors.top: gpsLatitude.bottom
        anchors.right: parent.right
        anchors.margins: 0
        anchors.topMargin: 0

        width: gpsLatitude.width
        height: gpsLatitude.height

        color: Qt.rgba(1, 1, 1, 0.85)

        Text {
            id: gpsLongitudeText;
            anchors.centerIn: parent
            text: qsTr("No GPS");
        }
    }

    // Information about geographic coordinates of the last clicked point
    Rectangle {
        id: selectedPointInformation

        anchors.top: gpsLongitude.bottom
        anchors.right: parent.right
        anchors.margins: 0

        width: selectedPointText.paintedWidth + 20
        height: selectedPointText.paintedHeight + 10

        color: Qt.rgba(1, 1, 1, 0.85)
        visible: false

        Text {
            id: selectedPointText
            text: ""
            anchors.centerIn: parent
        }
    }

    // Toolbar
    ToolBar {
        width: 40
        height: 40
        anchors.bottom: treeScaleSlider.top
        anchors.right: parent.right
        visible: Backend.loadedSystem() === "Marteloscope"

        ColumnLayout {
            anchors.fill: parent
            spacing: 1

            ToolButton {
                icon.source: "qrc:/images/ui_legend.png"
                text: ""
                onClicked: {
                    Backend.setShowMapTreeSpeciesLegend();
                    mapView.redraw()
                }
            }
        }
    }

    // Tree scale
    Slider {
        id: treeScaleSlider
        orientation: Qt.Vertical
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 40 // fixed width for vertical slider
        height: 200 // control how tall the slider is

        from: 1
        to: 10
        stepSize: 1
        value: Backend.treeScale()  // initial value

        onValueChanged: {
            Backend.setTreeScale(value) // Update settings
            redraw();
        }
    }

    // Toolbar
    ToolBar {
        width: 40
        height: 40
        anchors.top: treeScaleSlider.bottom
        anchors.right: parent.right

        ColumnLayout {
            anchors.fill: parent
            spacing: 1

            ToolButton {
                icon.source: "qrc:/images/ui_refocus.png"
                text: ""
                enabled: Backend.isLocationAvailable
                onClicked: {
                    Backend.refocus(mapImageId);
                    mapView.redraw()
                }
            }
        }
    }

    // Error
    CustomPopup {
        id: idNotFoundDialog

        title: qsTr("Error")
        text: qsTr("Id not found.")

        buttons: [
            TextButton {
                text: qsTr("Ok")
                onClicked: {
                    idNotFoundDialog.close();
                }
            }
        ]
    }
}
